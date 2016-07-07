/*
	Test AIS Sensor Firing and Sampling
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/select.h>
#include <string.h>

#include "sample.h"
#include "hw.h"

// 1st: the measurement set up to use
// 2nd: number of samples to collect
// 3rd: usec between sample time
// 4th: usec to wait between firing and sampling 
static SAMPLE_t __sample;
SAMPLE_p capture_one ( int setup, int numSamples, int samplingPeriod, int waitTime )
{
	uint32_t t_start, t, tprev, v, vmax, idleLevel;
	int i, n;
	
	// sanity check
	if (numSamples > 1024)
		numSamples = 1024;
	if (waitTime < 0)
		waitTime = 15000; // 15 msec
	
	// first fire the sensor
	printf("\n--- firing the sensor and will sample at (1/%dusec) sampling rate after %.2f msec ... \n", samplingPeriod, waitTime/1000.);
	LIN_send_StartMeasurement(setup);
	for (t_start = HW_tick(), idleLevel=0, n=0; HW_tick() - t_start < waitTime - ADC_READ_TIME; n++) 
		idleLevel += ADC_read();
	if (n>0)
		idleLevel /= n;
	t_start = tprev = HW_tick();
	vmax = ADC_read();
	
	// Collect the samples
	for (i=0; i<numSamples; i++) {
		for (t=HW_tick(); HW_tick() - tprev < samplingPeriod - ADC_READ_TIME_HALF; ) {
			v = ADC_read();
			if (v > vmax)
				vmax = v;
		}
		// timestamp is average of t and tprev
		__sample.t_ofs[i] = ((t + tprev)>>1) - t_start;
		__sample.val[i] = vmax;
		tprev = HW_tick();
		vmax = ADC_read();
	}
	__sample.num = i;
	__sample.setup = setup;
	return (&__sample);
}

void usage (char * progname) {
	printf ("\nUsage:\n\t%s [-N <num-cycles>] [-n <num-samples>] [-T <cycle-period>] [-t <sample-timing>] [-o <filename>]\n", progname);
	printf ("\nDescription:\n\tRecords the sensor readings to a file.\n");
	printf ("\nOptions:\n"
			"\n\t-N <num-cycles>\n"
			"\t\tSpecify the number of sensing cycles to record.  Default: 1\n"
			"\n\t-n <num-samples>\n"
			"\t\tSpecify the number of samples per cycle to record.  Default: 400\n"
			"\n\t-T <cycle-period>\n"
			"\t\tSepcify the time (msec) between each cycle.  Default: 1000ms\n"
			"\n\t-t <sampled-period>\n"
			"\t\tSepcify the time (usec) between each sample.  Default: 50usec\n"
			"\n\t-o <filename>\n"
			"\t\tSpecify the output filename.  If not specified, no output file will be generated.\n"
			"\t\tIt is possible to use the special string 'DATETIME' in the filename to request\n"
			"\t\tfor the current datatime to replace this string.  Note that the program will\n"
			"\t\tautomatically add in a '.json' extension\n"
			"\t\tExample: '-o foo/bar/DATETIME' will cause an output file 20151216-180211.json\n"
			"\t\tto be generated in the directory foo/bar, if it is currently 2015/12/16 18:02:11\n"
			"\n"
			);
}

int handle_arg_number (char * argv)
{
	char * p;
	int r = strtol(argv, &p, 10);
	return (*p != '\0') ? -1 : r;
}

char * handle_arg_filename (char * fname, char * argv)
{
	char *p = strstr(argv, "DATETIME");
	if (fname)
		free(fname);
	fname = (char*)malloc(64+strlen(argv));
	if (p) {
		struct timeval tv;
		int n = p-argv, r;
		if (n > 0) {
			strncpy(fname, argv, n);
			fname[n] = '\0';
		}
		gettimeofday(&tv,NULL);
		strftime(fname+n, 32, "%Y%m%d-%H%M%S", localtime(&(tv.tv_sec)));
		p += 8;
		if (*p)
			strcat(fname, p);
	} else {
		strcpy(fname, argv);
	}
	return (fname);
}

#define __input_argument_error__(fmt...) \
	printf(fmt); \
	usage(argv[0]); \
	exit(1)

#define __chk_missing_argument__(s) \
	if (i+1 >= argc) { \
		__input_argument_error__("Missing %s.\n", s); \
	}

int main (int argc, char * argv[])
{
	int i, numc=1, nums=700, tusec=50, tmsec=1000, prev=0;
	SAMPLE_p sample;
	FILE * fp;
	char * fname = NULL;

	// parse input argument
	for (i=1; i<argc; i++) {
		if (strcmp(argv[i], "-N") == 0) {
			__chk_missing_argument__("number-of-cycles");
			if ((numc = handle_arg_number(argv[i+1])) < 0) {
				__input_argument_error__("Invalid number '%s' for '-n' (%d).\n", argv[i+1], numc);
			}
			i++;
			continue;
		}
		if (strcmp(argv[i], "-T") == 0) {
			__chk_missing_argument__("inter-cycle timing");
			if ((tmsec = handle_arg_number(argv[i+1])) < 0) {
				__input_argument_error__("Invalid timing '%s' for '-t' (%d).\n", argv[i+1], tmsec);
			}
			i++;
			continue;
		}
		if (strcmp(argv[i], "-n") == 0) {
			__chk_missing_argument__("number-of-samples");
			if ((nums = handle_arg_number(argv[i+1])) < 0) {
				__input_argument_error__("Invalid number '%s' for '-n' (%d).\n", argv[i+1], nums);
			}
			i++;
			continue;
		}
		if (strcmp(argv[i], "-t") == 0) {
			__chk_missing_argument__("inter-sample timing");
			if ((tusec = handle_arg_number(argv[i+1])) < 0) {
				__input_argument_error__("Invalid timing '%s' for '-t' (%d).\n", argv[i+1], tusec);
			}
			i++;
			continue;
		}
		if (strcmp(argv[i], "-o") == 0) {
			__chk_missing_argument__("output filename");
			fname =  handle_arg_filename(fname, argv[i+1]);
			strcat(fname, ".json");
			i++;
			continue;
		}
		if (strcmp(argv[i], "-h") == 0) {
			__input_argument_error__("How to use:\n");
		}
		__input_argument_error__("Unknown argument '%s'.\n", argv[i]);
	}

	// initialize
	if (HW_init() < 0)
		return (-1);
	if (fname) {
		fp = fopen(fname, "w");
		fprintf(fp, "[\n");		// open array
	}
	sleep(1);

	// start capturing loop
	for (i=0; i<numc; i++) {
		struct timeval tv;	
		fd_set rfds;
		
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		tv.tv_sec = tmsec / 1000;
		tv.tv_usec = tmsec % 1000;
		if (select(1, &rfds, NULL, NULL, &tv) > 0) {
			switch (fgetc(stdin)) {
				case 'Q': case 'q': case 27:
					break;
				default:
					break;
			}
		}
		sample = capture_one(i&0x3, nums, tusec, -1);
		SAMPLE_2_json(sample, stdout);
		if (fp) {
			if (prev)
				fprintf(fp, ", \n");
			else prev=1;	
			SAMPLE_2_json(sample, fp);
		}
	}
	
	// clean up
	HW_close();
	if (fp) {
		fprintf(fp, "\n]\n");		// close array
		fclose(fp);
	}
	
	return (0);
}

