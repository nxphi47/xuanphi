/*
	Test AIS Sensor Firing and Sampling
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "bcm2835.h"

// ------------------ LIN -----------------------

#define __serial_dev "/dev/ttyAMA0"

#define LIN_PIN_BREAK	18			// set to 0 to send break signal (header pin 12)
#define LIN_PIN_TX_EN	4			// Enable TX (Header pin 7)
#define LIN_PIN_RX_EN	17			// Eanble RX (Header pin 11)
#define LIN_PIN_TX		14			// TX0 on GPIO (Header pin 8) 
#define LIN_PIN_RX		15			// RX0 on GPIO (Header pin 10)

// ------------------- UDP -----------------------
#define SERVER "10.80.43.121"
#define PORT 5558
#define BUF 16384

static uint32_t __sys_tick__(void) {
	volatile uint32_t* paddr = bcm2835_st + BCM2835_ST_CLO/4;
	return *paddr; 
}
static uint32_t __gpio_level__(void) {
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPLEV0/4;
	return *paddr; 
}
static void __gpio_set__(uint32_t mask) {
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPSET0/4;
	*paddr = mask; 
}
static void __gpio_clr__(uint32_t mask) {
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPCLR0/4;
	*paddr = mask; 
}
static void __gpio_clr_n_set__(uint32_t clr, uint32_t set) {
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPCLR0/4;
	*paddr = clr; 
	paddr = bcm2835_gpio + BCM2835_GPSET0/4;
	*paddr = set;
}

#define __LIN_SET_RX()	\
	__gpio_clr_n_set__( /*CLR*/1<<LIN_PIN_TX_EN , /*SET*/(1<<LIN_PIN_RX_EN)|(1<<LIN_PIN_BREAK))
	
#define __LIN_SET_TX()	\
	__gpio_clr_n_set__( /*CLR*/1<<LIN_PIN_RX_EN , /*SET*/(1<<LIN_PIN_TX_EN)|(1<<LIN_PIN_BREAK))

#define __LIN_SET_BRK()	\
	__gpio_clr__((1<<LIN_PIN_TX_EN) | (1<<LIN_PIN_RX_EN) | (1<<LIN_PIN_BREAK))

static int __serial_fd = -1;
static struct termios __serial_org_termios;

/* Initialize the LIN interface
 */
int LIN_init (void)
{
	struct termios s, s1;
	int r;
	speed_t b;
	
	if (__serial_fd > 0)	// already initialized
		return (0);
	
	// configure the GPIO
    bcm2835_gpio_fsel(LIN_PIN_TX, BCM2835_GPIO_FSEL_ALT0); /* TX0 */
    bcm2835_gpio_fsel(LIN_PIN_RX, BCM2835_GPIO_FSEL_ALT0); /* RX0 */
    bcm2835_gpio_fsel(LIN_PIN_TX_EN, BCM2835_GPIO_FSEL_OUTP); /* TX-Enable */
    bcm2835_gpio_fsel(LIN_PIN_RX_EN, BCM2835_GPIO_FSEL_OUTP); /* RX-Enable */
    bcm2835_gpio_fsel(LIN_PIN_BREAK, BCM2835_GPIO_FSEL_OUTP); /* BREAK */
	
	// set default signal
	__LIN_SET_RX();
	
	// get the original term attribute
	__serial_fd = open(__serial_dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if (__serial_fd <= 0) {
		perror (__serial_dev);
		return (-1);
	}
	if (tcgetattr(__serial_fd, &__serial_org_termios) < 0) {
		close(__serial_fd);
		__serial_fd = -1;
		perror ("tcgetattr()");
		return (-1);
	}
	memcpy(&s, &__serial_org_termios, sizeof(struct termios));
	
	// set up new termios
	s.c_iflag = IGNBRK | IGNPAR | IGNCR;
	s.c_oflag = 0; 
	s.c_cflag = CS8 | CLOCAL | CREAD;
	s.c_lflag = 0;
	s.c_cc[VMIN] = 0;
	s.c_cc[VTIME] = 100;

	// setspeed
	cfsetispeed(&s, B19200);
	cfsetospeed(&s, B19200);
	r = tcsetattr(__serial_fd, TCSANOW, &s);
	if (r<0)
		perror("tcsetattr()");
	tcflush(__serial_fd, TCIOFLUSH);
	
	r = tcgetattr(__serial_fd, &s1);
	if (r<0)
		perror("tcgetattr()");
	b = cfgetospeed(&s1);
	switch (b) {
		case B9600: printf("speed is 9600\n"); break;
		case B4800: printf("speed is 4800\n"); break;
		case B19200: printf("speed is 19200\n"); break;
		case B38400: printf("speed is 38400\n"); break;
		default: printf("some strange speed: 0x%x\n", b); break;
	}
	return (0);
}

/* Close the LIN interface
 */
void LIN_close (void)
{
	if (__serial_fd > 0) {
		tcsetattr(__serial_fd, TCSANOW, &__serial_org_termios);
		bcm2835_gpio_fsel(LIN_PIN_TX, BCM2835_GPIO_FSEL_INPT); /* TX0 */
		bcm2835_gpio_fsel(LIN_PIN_RX, BCM2835_GPIO_FSEL_INPT); /* RX0 */
		bcm2835_gpio_fsel(LIN_PIN_TX_EN, BCM2835_GPIO_FSEL_INPT); /* TX-Enable */
		bcm2835_gpio_fsel(LIN_PIN_RX_EN, BCM2835_GPIO_FSEL_INPT); /* RX-Enable */
		bcm2835_gpio_fsel(LIN_PIN_BREAK, BCM2835_GPIO_FSEL_INPT); /* BREAK */
		close(__serial_fd);
		__serial_fd = -1;
	}
}

// the PID look up table
static unsigned char __lin_pid_lut__[64] = {
	0x80, 0xC1, 0x42, 0x03, 0xC4, 0x85, 0x06, 0x47,
	0x08, 0x49, 0xCA, 0x8B, 0x4C, 0x0D, 0x8E, 0xCF,
	0x50, 0x11, 0x92, 0xD3, 0x14, 0x55, 0xD6, 0x97,
	0xD8, 0x99, 0x1A, 0x5B, 0x9C, 0xDD, 0x5E, 0x1F,
	0x20, 0x61, 0xE2, 0xA3, 0x64, 0x25, 0xA6, 0xE7,
	0xA8, 0xE9, 0x6A, 0x2B, 0xEC, 0xAD, 0x2E, 0x6F,
	0xF0, 0xB1, 0x32, 0x73, 0xB4, 0xF5, 0x76, 0x37,
	0x78, 0x39, 0xBA, 0xFB, 0x3C, 0x7D, 0xFE, 0xBF
};

#define __SERIAL_SEND_BYTE(B) {\
	unsigned char byte = B;\
	if (write (__serial_fd, &byte, 1) < 1)\
		perror("write()");\
}

#define __WAIT_ONE_BIT(__T) \
	for (__T=__sys_tick__(); __sys_tick__() - __T < 52; );

#define __SERIAL_SEND_BREAK_SYNC(START_TIME) {\
	__LIN_SET_BRK();\
	START_TIME = __sys_tick__();\
	bcm2835_delayMicroseconds(500);\
	for (; __sys_tick__() - START_TIME < 676; );\
	tcflush(__serial_fd, TCIOFLUSH);\
	__LIN_SET_TX();\
	__WAIT_ONE_BIT(START_TIME);\
	START_TIME = __sys_tick__();\
	__SERIAL_SEND_BYTE(0x55);\
}

#define __SEND_COMPLETE(START_TIME, NUMBYTE) {\
	unsigned char byte;\
	for (byte=1; byte > 0; )\
		ioctl(__serial_fd, TIOCOUTQ, &byte);\
	for (;__sys_tick__() - START_TIME < (NUMBYTE+1)*521;);\
	__LIN_SET_RX();\
}

/* Fire the AIS sensor by sending an LIN StartMeasurement frame */
void LIN_send_StartMeasurement (int measurementSetup)
{
	uint32_t start;
	unsigned char m, chksum;
	
	switch (measurementSetup & 0x03) {
		case 0:
			printf("Sending StartMeasurement using Setup #0 ...\n");
			__SERIAL_SEND_BREAK_SYNC(start);
			__SERIAL_SEND_BYTE(0x80);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0x00);
			__SERIAL_SEND_BYTE(0x2a);
			__SEND_COMPLETE(start, 5);
			break;
		case 1:
			printf("Sending StartMeasurement using Setup #1 ...\n");
			__SERIAL_SEND_BREAK_SYNC(start);
			__SERIAL_SEND_BYTE(0x80);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0x01);
			__SERIAL_SEND_BYTE(0x29);
			__SEND_COMPLETE(start, 5);
			break;
		case 2:
			printf("Sending StartMeasurement using Setup #2 ...\n");
			__SERIAL_SEND_BREAK_SYNC(start);
			__SERIAL_SEND_BYTE(0x80);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0x02);
			__SERIAL_SEND_BYTE(0x28);
			__SEND_COMPLETE(start, 5);
			break;
		case 3:
			printf("Sending StartMeasurement using Setup #3 ...\n");
			__SERIAL_SEND_BREAK_SYNC(start);
			__SERIAL_SEND_BYTE(0x80);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0xaa);
			__SERIAL_SEND_BYTE(0x03);
			__SERIAL_SEND_BYTE(0x27);
			__SEND_COMPLETE(start, 5);
			break;		
	}
}

void test_lin_firing (void)
{
	fd_set rfds;
	struct timeval tv;	
	int r;
	unsigned loop;
	
	for (loop=0;;loop++) {
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		tv.tv_sec=0;
		tv.tv_usec=500000;
		r = select(1, &rfds, NULL, NULL, &tv);
		if (r>0)
			break;
		LIN_send_StartMeasurement(loop);
	}
}

// ------------------- ADC -----------------------

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

/* ------------------------------ Capture Logic -------------------------------*/

typedef struct __sample_t__ {
	uint32_t t_ofs[1024];
	uint32_t val[1024];
	uint32_t num;
	int setup;
} SAMPLE_t, * SAMPLE_p;

// 1st: the measurement set up to use
// 2nd: number of samples to collect
// 3rd: usec between sample time
// 4th: usec to wait between firing and sampling 
static SAMPLE_t __sample;
SAMPLE_p capture_one ( int setup, int numSamples, int samplingPeriod, int waitTime )
{
	uint32_t t_start, t, v, vmax, idleLevel;
	int i, n;
	
	// sanity check
	if (numSamples > 1024)
		numSamples = 1024;
	if (waitTime < 0)
		waitTime = 15000; // 15 msec
	
	// first fire the sensor
	printf("\n--- firing the sensor and will sample at (1/%dusec) sampling rate after %.2f msec ... \n", samplingPeriod, waitTime/1000.);
	LIN_send_StartMeasurement(setup);
	for (t_start = __sys_tick__(), idleLevel=0, n=0; __sys_tick__() - t_start < waitTime; n++) 
		idleLevel += ADC_read();
	if (n>0)
		idleLevel /= n;		
	//bcm2835_delayMicroseconds(waitTime);
	t_start = __sys_tick__();
	
	// Collect the samples
	for (i=0; i<numSamples; i++) {
		for (t=__sys_tick__(), vmax=idleLevel-10; __sys_tick__() - t < samplingPeriod; ) {
			v = ADC_read();
			if (v > vmax)
				vmax = v;
		}
		__sample.t_ofs[i] = t - t_start;
		__sample.val[i] = vmax;
	}
	__sample.num = i;
	__sample.setup = setup;
	return (&__sample);
}

// convert sensed samples to a JSON object
void record_json ( SAMPLE_p s, FILE * f )
{
	int i;
	
	fprintf (f, "{ \"setup\": %d, \n  \"timing\": [ %.3f", s->setup, s->t_ofs[0]/1000.);
	for (i=1; i<s->num; i++) {
		if ((i % 10) == 0)
			fprintf (f, "\n\t");
		fprintf (f, ",  %.3f", s->t_ofs[i]/1000.);
	}
	fprintf (f, "],\n  \"envelop\": [ %u", s->val[0]);
	for (i=1; i<s->num; i++) {
		if ((i % 10) == 0)
			fprintf (f, "\n\t");
		fprintf (f, ",  %u", s->val[i]);
	}
	fprintf (f, "]\n},");
}

// realtime sending message
void real_time (SAMPLE_p s, int fd, int serverlen, struct sockaddr_in serveraddr)
{
	int i, cx;
	char buf[BUF];
	bzero(buf, BUF);

	printf ("Start sending\n");

//#define DEBUG
#ifdef DEBUG	
	int array[] = {940,  940,  940,  942,  940,  940,  939,  941,  944,  942,  940,  940,  940,  940,  942,  940,  939,  944,  943,  941,  942,  943,  942,  942,  944,  940,  941,  939,  940,  940,  942,  940,  942,  943,  947,  940,  940,  949,  945,  940,  948,  941,  940,  945,  945,  2047,  2177,  1894,  1862,  1755,  1728,  2195,  1520,  1596,  1588,  1590,  1752,  1852,  1763,  1553,  2216,  2118,  2224,  1541,  1641,  1364,  1351,  1283,  1184,  1041,  1080,  1108,  1052,  1084,  1068,  1052,  1040,  1015,  1018,  996,  1014,  982,  991,  987,  984,  971,  972,  958,  967,  957,  961,  959,  961,  947,  945,  947,  939,  937,  939,  941,  942,  944,  939,  940,  939,  939,  940,  940,  942,  939,  938,  939,  937,  939,  940,  939,  939,  937,  938,  940,  940,  938,  938,  939,  938,  939,  939,  940,  937,  940,  939,  940,  939,  940,  939,  937,  938,  940,  940,  942,  941,  940,  940,  947,  942,  940,  942,  941,  942,  940,  940,  937,  938,  942,  938,  939,  940,  940,  940,  941,  944,  939,  940,  943,  944,  941,  937,  937,  940,  939,  938,  941,  943,  940,  938,  940,  939,  940,  938,  937,  943,  941,  940,  937,  943,  940,  939,  940,  943,  939,  938,  939,  939,  940,  938,  937,  943,  944,  942,  939,  939,  938,  939,  939,  939,  940,  938,  940,  941,  940,  939,  940,  940,  940,  940,  944,  942,  944,  944,  940,  941,  940,  944,  940,  943,  948,  942,  944,  940,  944,  940,  942,  941,  946,  943,  944,  942,  943,  940,  940,  940,  940,  941,  940,  940,  940,  941,  951,  969,  1022,  1028,  1036,  1028,  945,  944,  943,  945,  944,  942,  940,  940,  940,  939,  940,  940,  940,  940,  939,  944,  941,  942,  943,  940,  940,  940,  940,  942,  942,  940,  940,  940,  940,  942,  941,  942,  942,  940,  939,  940,  942,  940,  940,  942,  940,  940,  941,  939,  941,  939,  939,  940,  940,  940,  940,  940,  940,  940,  940,  941,  940,  940,  944,  944,  944,  944,  942,  941,  940,  940,  942,  941,  940,  940,  940,  942,  940,  940,  944,  944,  943,  940,  942,  941,  940,  941,  942,  942,  940,  940,  941,  943,  942,  940,  940,  942,  941,  940,  940,  944,  943,  945,  943,  944,  944,  939,  942,  939,  942,  940,  941,  940,  940,  940,  941,  940,  938,  940,  944,  939,  942,  938,  941,  940,  944,  940,  943,  943,  945,  941,  941,  939,  942,  943,  951,  940,  944,  944,  948,  949,  947,  948,  942,  940,  945,  938,  945,  940,  943,  939,  949,  948,  971,  944,  939,  944,  940,  944,  943,  942,  939,  939,  939,  938,  941,  939,  942,  939,  946,  940,  937,  944,  940,  942,  940,  941,  940,  939,  938,  937,  940,  940,  938,  937,  940,  940,  940,  940,  939,  943,  940,  942,  939,  940,  939,  938,  939,  939,  940,  937,  940,  940,  940,  940,  939,  940,  940,  937,  940,  939,  939,  937,  939,  938,  939,  938,  939,  939,  940,  937,  939,  939,  939,  940,  939,  937,  938,  939,  939,  939,  937,  939,  941,  942,  947,  937,  940,  950,  943,  939,  938,  939,  942,  940,  937,  937,  938,  940,  940,  938,  939,  940,  937,  940,  948,  942,  940,  940,  945,  942,  940,  939,  939,  940,  939,  939,  939,  940,  940,  940,  939,  940,  941,  939,  945,  941,  938,  939,  937,  939,  938,  940,  942,  943,  939,  939,  945,  944,  942,  941,  940,  939,  939,  940,  939,  940,  940,  940,  943,  942,  939,  940,  939,  940,  941,  938,  938,  942,  939,  944,  944,  943,  940,  942,  940,  939,  940,  940,  943,  942,  945,  944,  950,  941,  944,  942,  946,  947,  946,  944,  940,  944,  940,  940,  940,  941,  940,  940,  943,  942,  939,  940,  941,  944,  940,  940,  943,  942,  941,  941,  942,  942,  942,  941,  942,  942,  942,  942,  941,  942,  941,  943,  943,  942,  943,  942,  942,  940,  942,  940,  944,  943,  940,  944,  940,  942,  942,  941,  943,  944,  944,  943,  944,  943,  943,  942,  940,  942,  942,  940,  941,  942,  940,  940,  942,  941,  942,  942,  940,  940,  943,  942,  942,  940,  940,  940,  944,  942,  948,  944,  944,  945,  942,  941,  940,  939,  941,  944,  945,  945,  943,  940,  940,  943,  942,  943,  943,  940,  940,  942,  942,  942,  944,  944,  944,  943,  940,  942,  943,  942,  942,  940,  941,  939,  940,  940,  943,  942,  940,  943,  942};
#endif	
	sprintf(buf, "MATRIX:{ \"setup\": %d,  \"timing\": [ %.3f", s->setup, s->t_ofs[0]/1000.);

	for (i=1; i<s->num; i++) {
		sprintf (buf, "%s,  %.3f", buf, s->t_ofs[i]/1000.);
	}

	sprintf(buf, "%s],  \"envelop\": [ %u", buf, s->val[0]);

	for (i=1; i<s->num; i++) {
#ifdef DEBUG
		sprintf (buf, "%s, %d", buf, array[i]);
#else
		sprintf (buf, "%s,  %u", buf, s->val[i]);
#endif		
	}

	sprintf (buf, "%s]}", buf);

	int n = sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
	if (n < 0)
		printf ("ERROR in sending message.\n");

}

int main_test (int argc, char * argv[])
{
	int i, num, usec=1000000;
	uint32_t t, s;
	
	if (bcm2835_init() != 1) {
		printf ("bcm2835_init() failed.\n");
		return (-1);
	}

	LIN_init();
	sleep(1);
	test_lin_firing();
	sleep(1);
	LIN_close();
	
	return (0);
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

	if (bcm2835_init() != 1) {
		printf ("bcm2835_init() failed.\n");
		return (-1);
	}
	
	// prepare UDP socket
	int sockfd, n;
	int serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	
	// create socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf ("ERROR opening socket.\n");
		return (-1);
	}
	
	// build server internet address
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVER);
	serveraddr.sin_port = htons(PORT);
	serverlen = sizeof(serveraddr);

	ADC_init();
	LIN_init();	
	
	fp =  (fname) ? fopen(fname, "w") : NULL;
	sleep(1);

//	for (i=0; i<numc; i++) {
	while (1) {
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
		real_time(sample, sockfd, serverlen, serveraddr);
/**		record_json(sample, stdout);	
		if (fp) {
			if (prev)
				fprintf(fp, ", \n");
			else prev=1;	
			record_json(sample, fp);
		}
*/	}
	
	LIN_close();
	ADC_fini();
	
/**	if (fp) {
		fprintf(fp, "]\n");
		fclose(fp);
	}
*/	
	return (0);
}

