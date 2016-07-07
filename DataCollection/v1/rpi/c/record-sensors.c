#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "rpi-gpio.h"

void usage (char * progname) {
	printf ("\nUsage:\n\t%s [-n <num-cycles>] [-s <enabled-sensors>] [-t <timings>] [-o <filename>]\n", progname);
	printf ("\nDescription:\n\tRecords the sensor readings to a file.\n");
	printf ("\nOptions:\n"
			"\n\t-n <num-cycles>\n"
			"\t\tSpecify the number of sensing cycles to record.  Default: 1\n"
			"\t\tIn one sensing cycle, each of the enabled sensors will be sequentially triggered\n"
			"\t\tto perform ranging and sampling.\n"
			"\n\t-s <enabled-sensors>\n"
			"\t\tSpecify which of sensors will be triggered.  There are 4 sensors, numbered 0, 1,\n"
			"\t\t2, 3.  Specify the enabled sensors by their numbers separated by comma (or no\n"
			"\t\tseparation).\n"
			"\t\tExample: -s 01,3  will enable sensors #0, #1, and #3.\n"
			"\t\tDefault: only sensor #0 is enabled.\n"
			"\n\t-t <timings>\n"
			"\t\tSepcify the timings within a sensing cycle.  <timings> should be given as 4-tuple\n"
			"\t\tintegral numbers, e.g. 17,23,50,100.\n"
			"\n\t\tThe first timing is the ranging wait time.  This specifies the number of msec\n"
			"\t\tto wait after a sensor is triggered for ranging before sampling will start.\n"
			"\t\tDefault is 17msec.\n"
			"\n\t\tThe second timing is the length of sampling period.  This specifies the length\n"
			"\t\tof time in msec to perform sampling of the analog envelop.  Default is 23msec.\n"
			"\n\t\tThe third timing is the minimum interval between successive ranging of two\n"
			"\t\ttwo sensors (e.g. between sensor #1 and sensor #2).  Default is 40msec.\n"
			"\n\t\tThe forth timing is the minimum interval between successive ranging of the same\n"
			"\t\tsensor (e.g. between the the first time sensor #0 is triggered and the second \n"
			"\t\ttime sensor #0 is triggered again).  Default is 100msec (minimum).\n"
			"\n\t-o <filename>\n"
			"\t\tSpecify the output filename.  If not specified, no output file will be generated.\n"
			"\t\tIt is possible to use the special string 'DATETIME' in the filename to request\n"
			"\t\tfor the current datatime to replace this string.\n"
			"\t\tExample: -o foo/bar/DATETIME.json  will cause an output file 20151216-180211.json\n"
			"\t\tto be generated in the directory foo/bar, if it is currently 2015/12/16 18:02:11\n");
			
	printf("NOTE: Ignore above description of '-s'.  Current version will only use sensor #0.\n");
}

int handle_arg_cycle (char * argv)
{
	char * p;
	int r = strtol(argv, &p, 10);
	return (*p != '\0') ? -1 : r;
}

int handle_arg_sensors (RPI_CB_p rpi, char * argv)
{
	rpi->sensors[0].enabled = (strchr(argv, '0')) ? 1 : 0;
	rpi->sensors[1].enabled = (strchr(argv, '1')) ? 1 : 0;
	rpi->sensors[2].enabled = (strchr(argv, '2')) ? 1 : 0;
	rpi->sensors[3].enabled = (strchr(argv, '3')) ? 1 : 0;
	return (0);
}

int handle_arg_timings (RPI_CB_p rpi, char * argv)
{
	char *p, *sp = argv;
	int t_r, t_s, t_n, t_c;

	// get T_ranging
	t_r = strtol(sp=argv, &p, 10);
	if ((t_r <= 0) || (*p != ','))
		return (-1);

	// get T_sampling
	t_s = strtol(sp=p+1, &p, 10);
	if ((t_s <= 0) || (*p != ','))
		return (-2);
	
	// get T_next
	t_n = strtol(sp=p+1, &p, 10);
	if ((t_s <= 0) || (*p != ','))
		return (-3);

	// get T_cycle
	t_c = strtol(sp=p+1, &p, 10);
	if ((t_s <= 0) || (*p != '\0'))
		return (-4);
		
	rpi->usec_ranging = t_r * 1000;
	rpi->usec_sampling = t_s  * 1000;
	rpi->usec_next = t_n  * 1000;
	rpi->usec_cycle = t_c * 1000;
	return (0);
}

char * handle_arg_filename (char * fname, char * argv)
{
	char *p = strstr(argv, "DATETIME");
	if (fname)
		free(fname);
	if (p) {
		struct timeval tv;
		int n = p-argv, r;
		fname = (char*)malloc(32+strlen(argv));
		if (n > 0) {
			strncpy(fname, argv, n);
			fname[n] = '\0';
		}
		gettimeofday(&tv,NULL);
		strftime(fname+n, 32, "%Y%m%d-%H%M%S", localtime(&(tv.tv_sec)));
		p += 8;
		if (*p)
			strcat(fname, p);
		return (fname);
	} else {
		return strdup(argv);
	}
}
		

#define __input_argument_error__(fmt...) \
	printf(fmt); \
	usage(argv[0]); \
	rpigpio_fini(); \
	exit(1)

#define __chk_missing_argument__(s) \
	if (i+1 >= argc) { \
		__input_argument_error__("Missing %s.\n", s); \
	}

int main (int argc, char * argv[])
{
	RPI_CB_p rpi = rpigpio_init();
	int i=1, ret, num_cycle=1;
	char * fname = NULL;
	const char * buf;
	SHISTORY_p pSam;
	FILE * outf = NULL;

	while (i<argc) {
		if (strcmp(argv[i], "-t") == 0) {
			__chk_missing_argument__("timing parameters");
			ret = handle_arg_timings(rpi, argv[i+1]);
			if (ret<0) {
				__input_argument_error__("Invalid timing parameters '%s' (%d).\n", argv[i+1], ret);
			}
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "-s") == 0) {
			__input_argument_error__("Multi-sensor ranging not yet supported.\n");
			__chk_missing_argument__("sensor parameters");
			handle_arg_sensors(rpi, argv[i+1]);
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "-n") == 0) {
			__chk_missing_argument__("number of cycles");
			num_cycle = handle_arg_cycle(argv[i+1]);
			if (num_cycle<1) {
				__input_argument_error__("Invalid number of cycles '%s' (%d).\n", argv[i+1], ret);
			}
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "-o") == 0) {
			__chk_missing_argument__("output filename");
			fname =  handle_arg_filename(fname, argv[i+1]);
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "-h") == 0) {
			__input_argument_error__("How to use:\n");
		}	
		__input_argument_error__("Unknown argument '%s'.\n", argv[i]);
	}
	// always use sensor #0 only
	rpi->sensors[0].enabled = 1;
	
	printf("RPiCB=%s\n", rpigpio_print());
	for (i=0; i<num_cycle; i++) {
		printf("\nRanging cycle %d:\n", i);
		pSam = rpigpio_sensor_range_and_sample (rpi->sensors);
		rpigpio_sensor_update_history(rpi->sensors, 0);
	}
	
	if (fname)
		outf = fopen(fname, "w");

	for (pSam = rpi->sensors->prev; pSam; pSam = pSam->next) {
		buf = rpigpio_history_print(pSam);
		puts(buf);
		if (outf)
			fputs(buf, outf);
	}
	if (fname)
		if (outf) {
			fclose(outf);
			printf("Output file '%s' created.\n\n", fname);
		} else
			printf("Unable to create output file '%s'.\n\n", fname);
	
	rpigpio_fini();	
	return (0);
}
