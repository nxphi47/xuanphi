#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "rpi-gpio.h"

void usage (char * progname) {
	printf ("\nUsage:\n\t%s [-n <num-cycles>] [-s <enabled-sensors>] [-t <timings>] [-o <filename>] [-c <prefix>] [-a] [-oc <prefix>]\n", progname);
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
			"\t\tSepcify the timings within a sensing cycle.  <timings> should be given as 5-tuple\n"
			"\t\tintegral numbers, e.g. 17,33,50,500,48\n"
			"\n\t\tThe first timing is the ranging wait time.  This specifies the number of msec\n"
			"\t\tto wait after a sensor is triggered for ranging before sampling will start.\n"
			"\t\tDefault is 17msec.\n"
			"\n\t\tThe second timing is the length of sampling period.  This specifies the length\n"
			"\t\tof time in msec to perform sampling of the analog envelop.  Default is 33msec.\n"
			"\n\t\tThe third timing is the minimum interval between successive ranging of two\n"
			"\t\ttwo sensors (e.g. between sensor #1 and sensor #2) in msec.  Default is 40msec.\n"
			"\n\t\tThe forth timing is the minimum interval between successive ranging of the same\n"
			"\t\tsensor (e.g. between the the first time sensor #0 is triggered and the second \n"
			"\t\ttime sensor #0 is triggered again) in msec.  Default is 500msec (minimum=100).\n"
			"\n\t\tThe fifth timing is the interval between successive sampling in usec.\n"
			"\t\tDefault is 48usec.\n"
			"\n\t-o <filename>\n"
			"\t\tSpecify the output filename.  If not specified, no output file will be generated.\n"
			"\t\tIt is possible to use the special string 'DATETIME' in the filename to request\n"
			"\t\tfor the current datatime to replace this string.  Note that the program will\n"
			"\t\tautomatically add in a '.json' extension\n"
			"\t\tExample: '-o foo/bar/DATETIME' will cause an output file 20151216-180211.json\n"
			"\t\tto be generated in the directory foo/bar, if it is currently 2015/12/16 18:02:11\n"
			"\n\t-a\n"
			"\t\tDo sampling for all sensors.\n"
			"\t\tUsually, we only sample the sensor that does the ranging.  With the -a option, we\n"
			"\t\tforce the ADC to sample all sensors, even those that are not enabled.\n"
			"\n\t-c <prefix>\n"
			"\t\tCapture image.\n"
			"\t\tAt every ranging, ask the RPi to capture an image and save it.\n"
			"\t\tResulting image filename will be the specified prefix and the sensor data id as\n"
			"\t\tsuffix, and .png as extension.\n"
			"\t\tIt is possible to use the special string 'DATETIME' in the filename to request\n"
			"\t\tfor the current datatime to replace this string.\n"
			"\t\tExample: '-c foo/bar/DATETIME' will cause an output file 20151216-180211-1000.png\n"
			"\t\tto be generated in the directory foo/bar, if it is currently 2015/12/16 18:02:11\n"
			"\t\tfor the sanor data 1000.\n"
			"\n\t-oc <prefix>\n"
			"\t\tSpecify -o and -c at the same time with the same prefix.\n"
			"\t\tIt will be equivalent to specifying\n\n\t\t\t-o <prefix> -c <prefix>\n"
			"\n"
			);
			
//	printf("NOTE: Ignore above description of '-s'.  Current version will only use sensor #0.\n");
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
	int t_r, t_s, t_n, t_c, t_us;

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
	if ((t_s <= 0) || (*p != ','))
		return (-4);
	if (t_c < 100)
		t_c = 100;
		
	// get T_sample
	t_us = strtol(sp=p+1, &p, 10);
	if ((t_us <= 0) || (*p != '\0'))
		return (-5);
		
	rpi->usec_ranging = t_r * 1000;
	rpi->usec_sampling = t_s  * 1000;
	rpi->usec_next = t_n  * 1000;
	rpi->usec_cycle = t_c * 1000;
	rpi->usec_sample = t_us;
	return (0);
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
	rpigpio_fini(); \
	exit(1)

#define __chk_missing_argument__(s) \
	if (i+1 >= argc) { \
		__input_argument_error__("Missing %s.\n", s); \
	}

int main (int argc, char * argv[])
{
	RPI_CB_p rpi = rpigpio_init();
	int i=1, j=0, k=0, ret, num_cycle=1, all=0, histid=0;
	char * fname = NULL;
	char * img_prefix = NULL;
	const char * buf;
	SENSOR_p sen;
	SHISTORY_p pSam;
	FILE * outf = NULL;

	rpi->sensors[0].enabled = 1;		// default enable sensor #0

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
			//__input_argument_error__("Multi-sensor ranging not yet supported.\n");
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
			strcat(fname, ".json");
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "-c") == 0) {
			__chk_missing_argument__("image prefix");
			img_prefix =  handle_arg_filename(img_prefix, argv[i+1]);
			if (rpicam_init() == 0)
				rpi->ctrl_flags |= RPICB_ENABLE_CAM;
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "-oc") == 0) {
			__chk_missing_argument__("output/image prefix");
			fname =  handle_arg_filename(fname, argv[i+1]);
			if (img_prefix)
				free(img_prefix);
			img_prefix = strdup(fname);
			strcat(fname, ".json");
			if (rpicam_init() == 0)
				rpi->ctrl_flags |= RPICB_ENABLE_CAM;
			i += 2;
			printf("arg: -oc = fname=%s, img_prefix=%s\n", fname, img_prefix);
			continue;
		}
		if (strcmp(argv[i], "-a") == 0) {
			all = 1;
			i ++;
			continue;
		}
		if (strcmp(argv[i], "-h") == 0) {
			__input_argument_error__("How to use:\n");
		}	
		__input_argument_error__("Unknown argument '%s'.\n", argv[i]);
	}
	// always use sensor #0 only
	// rpi->sensors[0].enabled = 1;

	// one round of warm-up ranging
	for (j=0, sen=rpi->sensors; j<NUM_SENSORS; j++, sen++)
		rpigpio_sensor_range_and_sample (sen);
	
	printf("RPiCB=%s\n", rpigpio_print());
	for (i=0; i<num_cycle; i++) {
		for (j=0, sen=rpi->sensors; j<NUM_SENSORS; j++, sen++) {
			histid = i * 10 + j;
			printf("\nRanging cycle %d: Sensor#%d, histid=%04d\n", i, j, histid);
			if (!all) {
				rpigpio_sensor_range_and_sample (sen);
				rpigpio_sensor_update_history_with_id(sen, 0, histid);
			} else {
				rpigpio_sensor_range_and_sample_all (sen);
				for (k=0; k<NUM_SENSORS; k++)
					rpigpio_sensor_update_history_with_id(rpi->sensors+k, 0, histid);
			}
			if (sen->enabled && (rpi->ctrl_flags & RPICB_ENABLE_CAM))
				rpicam_save_image(img_prefix, histid);
		}
	}
	
	if (fname) {
		outf = fopen(fname, "w");
		if (outf)
			fputs(rpigpio_print(), outf);
	}

	for (j=0, sen=rpi->sensors; j<NUM_SENSORS; j++, sen++) {
		for (pSam = sen->prev; pSam; pSam = pSam->next) {
			buf = rpigpio_history_print(pSam, j);
			if (buf) {
				puts(buf);
				if (outf)
					fputs(buf, outf);
			}
		}
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
