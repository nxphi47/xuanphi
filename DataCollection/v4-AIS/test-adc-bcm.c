/*
	Convenient APIs for Data Collection Circuit
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "bcm2835.h"

// ------------------- Test ADC -----------------------

// initialize the ADC
void ADC_init (void) 
{
	// MC3202 has a requirement of minimum 10kHz to maintain the stored charge for 1.2ms
	// we are sampling at ~48usec, and each sampling takes 3 bytes (24bits) of SPI communications
	// Hence we should set the speed at minimum 24/48e-6 = 500kHz
	// Let's play save and use 1.2MHz
	if (bcm2835_spi_begin() != 1) {
		printf ("bcm2835_spi_begin() fails\n");
		return;
	}
	bcm2835_spi_chipSelect(0);
	bcm2835_spi_setClockDivider(128);
	bcm2835_spi_setChipSelectPolarity(0,0);
}

// get sample from ADC
uint32_t ADC_read (void)
{
	uint32_t ret = 0;
	unsigned char buf[4];
	
	// MC3202 data format
	// tx:   | ST | SGL | ODD | MSB |
	// rx:                          | NUL | B11 | B10 | B9 | B8 | B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 | B1 | B2 | B3 |
	// We set the buffer accordingly:
	//             Byte 0         -> <-              Byte 1                  -> <-             Byte 2                ->
	// What we want is:                    <----------------------------------------------------------->
	buf[0] = 0x0d;  // ST=1 SGL=1 ODD=0 MSB=1
	buf[1] = 0x00;
	buf[2] = 0x00;
	bcm2835_spi_transfernb(buf, buf, 3);
	
	// printf ("ADC_read(): 0x%2x 0x%2x 0x%2x\n", buf[0], buf[1], buf[2]);
	
    return (((buf[1] & 0x7f) << 5) | ((buf[2] >> 3) & 0x1f));
}

// shutdown ADC
void ADC_fini (void) 
{
	bcm2835_spi_end();
}
    
int main (int argc, char * argv[])
{
	int i, num, usec=1000000;
	uint32_t t, s;
	
	if (argc<2) {
		printf ("Usage: %s <num> [usec]\n", argv[0]);
		printf ("\t<num>: number of sampled to get.\n");
		printf ("\t<usec>: number of microseconds to wait between samples. (default: %d)\n", usec);
		return (0);
	}
	num = atoi(argv[1]);
	if (argc > 2)
		usec = atoi(argv[2]);
	
	if (bcm2835_init() != 1) {
		printf ("bcm2835_init() failed.\n");
		return (-1);
	}
		
	ADC_init();
	for (i=0; i<num; i++) {
		bcm2835_delayMicroseconds(usec);
		t = bcm2835_st_read();
		s = ADC_read();
		printf ("tick=%010u, voltage=0x%03x\n", t, s);
	}
	
	return (0);
}

