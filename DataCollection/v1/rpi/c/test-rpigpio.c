#include <stdio.h>
#include <unistd.h>

#include "rpi-gpio.h"

void prompt (void) {
	char c;
	rpigpio_pins_show();	
	sleep(1);
	printf("-- Press any key to continue\n");
	read(0, &c, 1);
}

void test1 (void)
{
	RPI_CB_p rpi = rpigpio_init();
	SAMPLE data;
	
	rpigpio_pins_show();
	
	sleep(1);

	printf("\n-- Going to Start Ranging Sensor 0 ...\n");
	rpigpio_start_range(0);
	printf("-- Done.\n\n");

	prompt();

	printf("\n-- Going to Start Conversion in Sensor 0 ...\n");
	rpigpio_start_convert(0);
	printf("-- Done.\n\n");

	prompt();

	printf("\n-- Going to Start latching Sensor 0 ...\n");
	rpigpio_start_latch(0);
	printf("-- Done.\n\n");

	prompt();

	printf("\n-- Going to Start Reading ...\n");
	data = rpigpio_read_data();
	printf("-- Done data=%d.\n\n", data);
	rpigpio_pins_show();
	
	rpigpio_fini();
}

void test2 (void)
{
	RPI_CB_p rpi = rpigpio_init();
	SHISTORY_p hist;
	int i;
	
	rpi->usec_ranging = 15000;
	rpi->usec_sampling = 100000;
	rpi->usec_next = 100000;
	rpi->usec_cycle = 200000;
	rpi->sensors[0].enabled = 1;
	
	printf("-- RPi initialized.\n");
	prompt();
	
	for (i=0; i<10; i++) {
		printf("-- Start ranging %d\n", i);
		hist = rpigpio_sensor_range_and_sample (rpi->sensors);
		puts(rpigpio_history_print(hist));
	}
	
	rpigpio_fini();
}

int main (int argc, char * argv[])
{
	test2();
	return (0);
}
