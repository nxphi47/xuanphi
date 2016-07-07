/*
	Convenient APIs for Data Collection Circuit
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// for serial interface
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>
#include "bcm2835.h"


#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif


#define __serial_dev "/dev/ttyAMA0"

#define LIN_PIN_BREAK	18			// set to 0 to send break signal
#define LIN_PIN_TX_EN	4			// Enable TX
#define LIN_PIN_RX_EN	17			// Eanble RX
#define LIN_PIN_TX		14			// TX0 on GPIO (Header pin 8) 
#define LIN_PIN_RX		15			// RX0 on GPIO (Header pin 10)

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

/*
#define __LIN_SET_RX()	\
	bcm2835_gpio_clr(LIN_PIN_TX_EN); \
	bcm2835_gpio_set_multi((1<<LIN_PIN_RX_EN)|(1<<LIN_PIN_BREAK))
	
#define __LIN_SET_TX()	\
	bcm2835_gpio_clr(LIN_PIN_RX_EN); \
	bcm2835_gpio_set_multi((1<<LIN_PIN_TX_EN)|(1<<LIN_PIN_BREAK))

#define __LIN_SET_BRK()	\
	bcm2835_gpio_clr_multi((1<<LIN_PIN_TX_EN) | (1<<LIN_PIN_RX_EN) | (1<<LIN_PIN_BREAK))
*/

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

// Transmit a frame
// 	<id> is the contains only the first 6 bits: the P0 and P1 protection will be calculated by this function
//	<len> is the number of data bytes
//	<data> is the array of bytes to send
//	Checksum will be calculated by this function
void LIN_send_frame (unsigned char id, int len, unsigned char * data )
{
	unsigned char pid, sync=0x55;
	int byte, r;
	uint32_t chksum, t;
#ifdef __DEBUG__
	char debugstr[32];
#endif
	
	// the LIN protocol specifies a minimum 13-bit long dorminant pulse as a break signal,
	// followed by the start byte 0x55 to precede all frames.  
	// To get 13-bit, we have two methods:
	// (A) generate it using a dedicated GPIO pin (we use GPIO18 here)
	// (B) set the baudrate to 9600 = this will generate a 16-bit long break pulse
	
	// break the line -- using Method A
	debug("-- LIN_send_frame(): sending break ...\n");
	t = bcm2835_st_read();
	__LIN_SET_BRK();
	tcflush(__serial_fd, TCIFLUSH);

	// do calculations here
	pid = __lin_pid_lut__[id&0x3F];
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	for (byte=0; byte<len; byte++) 
		chksum += data[byte];
	chksum = ~(chksum + (chksum >> 8)) & 0xFF;

#ifdef __DEBUG__
	sprintf (debugstr, "0x%02x ", pid);
	for (byte=0; byte<len; byte++) 
		sprintf (debugstr + (byte+1)*6, "0x%02x  ", data[byte]);
	sprintf (debugstr + (byte+1)*6, "0x%02x  ", chksum);	
#endif

	// wait for break to finish
	// LIN requires 13 bit of 0 (Dorminant) to break the signal.
	// At 19.2kHz, that is 677.08usec of 0-bits
	bcm2835_delayMicroseconds(t + 677 - bcm2835_st_read());
	__LIN_SET_TX();
	// one bit of recesive as break delimiter
	bcm2835_delayMicroseconds(52);
	
	// send sync field and the frame and then chksum
	debug("-- LIN_send_frame(): sending frame [ %s ] ...\n", debugstr);
	r = write (__serial_fd, &sync, 1);
	if (r<0)
		perror("write\n");
	r = write (__serial_fd, &pid, 1);
	if (r<0)
		perror("write\n");
	for (byte=0; byte<len; ) {
		byte += write(__serial_fd, data+byte, 1);
	}
	write(__serial_fd, &chksum, 1);

	// wait for TX queue to be empty
	debug("-- LIN_send_frame(): waiting for transmission to complete ...\n");
	for (byte=1; byte > 0; ) {
		ioctl(__serial_fd, TIOCOUTQ, &byte);
	}

	// return the LIN bus back to idle state (recessive = 1)
	{
		char rbuf[100];
		int rr = read(__serial_fd, rbuf, 100);
		printf ("read() returns %d\n", rr);
		if (rr<0)
			perror("read()");
	}
	tcflush(__serial_fd, TCIFLUSH);
	__LIN_SET_RX();	
}


#define LIN_ERR_TIMEOUT		-1
#define LIN_ERR_CHKSUM		-2
#define LIN_ERR_SERIAL		-3

// Poll for a response frame
// 	<id> is the contains only the first 6 bits: the P0 and P1 protection will be calculated by this function
//	<len> is the number of data bytes expected
//	<data> is the array of bytes to received the data
// NOTE: <data> must be at least <len>+1 size.  The +1 is for receiving the checksum
// Function returns 0 on success, negative error codes on error
int LIN_poll_frame (unsigned char id, int len, unsigned char * data)
{
	unsigned char pid, sync=0x55;
	int byte, ret;
	uint32_t chksum, start;
	
	// break the line
	debug("-- LIN_poll_frame(): sending break ...\n");
	start = bcm2835_st_read();
	__LIN_SET_BRK();
	tcflush(__serial_fd, TCIFLUSH);

	// do calculations here
	pid = __lin_pid_lut__[id&0x3F];
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	
	// wait for break to finish
	// LIN requires 13 bit of 0 (Dorminant) to break the signal.
	// At 19.2kHz, that is 677.08usec of 0-bits
	bcm2835_delayMicroseconds(start + 677 - bcm2835_st_read());
	__LIN_SET_TX();
	// one bit of recesive as break delimiter
	bcm2835_delayMicroseconds(52);
	debug("-- LIN_poll_frame(): sending sync ...\n");

	// send sync field and the frame and then chksum
	write (__serial_fd, &sync, 1);
	write (__serial_fd, &pid, 1);

	debug("-- LIN_poll_frame(): sending %0x%x %0x%x...\n", sync, pid);

	// wait for TX queue to be empty
	for (byte=1; byte > 0; )
		ioctl(__serial_fd, TIOCOUTQ, &byte);

	// receive response
	tcflush(__serial_fd, TCIFLUSH);
	debug("-- LIN_poll_frame(): goining into receive mode ... \n");
	__LIN_SET_RX();	
	start = bcm2835_st_read();
	for (byte=0; byte<=len; ) {
		ret = read(__serial_fd, data+byte, len-byte+1);
		debug("-- LIN_poll_frame(): received %d byte ...\n", ret);
		if (ret < 0) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				usleep(1);
				continue;
			}
			perror ("-- LIN_Poll_frame() read() error");
			return (LIN_ERR_SERIAL);
		}
		byte += ret;
		if (bcm2835_st_read() > start + 50000) {
			// more than 50ms 
			fprintf (stderr, "-- LIN_poll_frame(): timeout waiting for response from slave.\n");
			return (LIN_ERR_TIMEOUT);
		}
	}
	// verify checksum
	for (byte=0; byte<len; byte++)
		chksum += data[byte];
	chksum = (chksum + (chksum >> 8)) & 0xFF;
	if (chksum != 0xFF) {
		fprintf (stderr, "-- LIN_poll_frame(): checksum failed.\n");
		return (LIN_ERR_CHKSUM);
	}
	return (0);
}

// Specifc Frames
#define LIN_ELMOS_START_MEASUREMENT				0x00
#define LIN_ELMOS_MEASURE_CONFIG_DATA2_WRITE	0x30
#define LIN_ELMOS_MEASURE_CONFIG_DATA2_READ		0x31
#define LIN_ELMOS_CONFIG_DATA_WRITE_ENABLE		0x38
#define LIN_ELMOS_CONFIG_DATA_READ_REQ			0x39

void LIN_send_StartMeasurement (int measurementSetup)
{
	unsigned char frame[3] = { 0xAA, 0xAA, 0x00 };
	frame[2] |= (measurementSetup & 0x03);
	LIN_send_frame (LIN_ELMOS_START_MEASUREMENT, 3, frame);
}

typedef struct __tag_lin_measurement_config_data2__ {
	unsigned char sID;		// slave ID
	unsigned char mlen[4];	// measurement length
	unsigned char blen[4];  // burst length
	unsigned char thresRising;	// Threshold slope rising
	unsigned char thresFalling;	// Threshold slope falling
} LIN_MEASURE_CFG_DATA2_t, * LIN_MEASURE_CFG_DATA2_p;

#define LIN_send_ConfigDataWriteEnable(SlaveID,Store,Wait) \
	frame[0] = SlaveID & 0x0F;\
	frame[1] = Store & 0x0F;\
	LIN_send_frame(LIN_ELMOS_CONFIG_DATA_WRITE_ENABLE, 2, frame);\
	bcm2835_delayMicroseconds(Wait)

void LIN_send_MeasurementConfig2Write ( const LIN_MEASURE_CFG_DATA2_p mcd2, int toEEPROM )
{
	char frame[8];
	
	LIN_send_ConfigDataWriteEnable(mcd2->sID, 0x00, 20000);	
	
	frame[0] = mcd2->sID & 0x0f;
	frame[1] = mcd2->mlen[0];
	frame[2] = mcd2->mlen[1];
	frame[3] = mcd2->mlen[2];
	frame[4] = mcd2->mlen[3];
	frame[5] = (mcd2->blen[0] & 0x0f) | ((mcd2->blen[1] << 4) & 0xf0);
	frame[6] = (mcd2->blen[2] & 0x0f) | ((mcd2->blen[3] << 4) & 0xf0);
	frame[7] = (mcd2->thresFalling & 0x0f) | ((mcd2->thresRising << 4) & 0xf0);
	LIN_send_frame (LIN_ELMOS_MEASURE_CONFIG_DATA2_WRITE, 8, frame);
	
	LIN_send_ConfigDataWriteEnable(mcd2->sID, 0x05, 20000);	
	if (toEEPROM) {
		LIN_send_ConfigDataWriteEnable(mcd2->sID, 0x50, 1600000);
	}
}

#define LIN_send_ConfigDataReadReq(SlaveID,Mem) \
	frame[0] = (Mem ? 0x10 : 0x00) | (SlaveID & 0x0F);\
	LIN_send_frame (LIN_ELMOS_CONFIG_DATA_READ_REQ, 1, frame); \
	bcm2835_delayMicroseconds(1000)

int LIN_send_MeasurementConfig2Read ( LIN_MEASURE_CFG_DATA2_p mcd2, int fromEEPROM )
{
	char frame[8];
	
	LIN_send_ConfigDataReadReq (mcd2->sID, fromEEPROM);
	if (LIN_poll_frame (LIN_ELMOS_MEASURE_CONFIG_DATA2_READ, 8, frame) == 0) {
		mcd2->mlen[0] = frame[1];
		mcd2->mlen[1] = frame[2];
		mcd2->mlen[2] = frame[3];
		mcd2->mlen[3] = frame[4];
		mcd2->blen[0] = frame[5] & 0x0F;
		mcd2->blen[1] = (frame[5] >> 4) & 0x0F;
		mcd2->blen[2] = frame[6] & 0x0F;
		mcd2->blen[3] = (frame[6] >> 4) & 0x0F;
		mcd2->thresFalling = frame[7] & 0x0F;
		mcd2->thresRising = (frame[7] >> 4) & 0x0F;
		bcm2835_delayMicroseconds(20000);
		return (0);
	}
	return (-1);
}


static void print_measurement_config (LIN_MEASURE_CFG_DATA2_p mcd2)
{
	printf ("Measurement Configuration for tslaveID = 0x%x\n", mcd2->sID);
	printf ("\tmLen[] = %3d, %3d, %3d, %3d\n", mcd2->mlen[0], mcd2->mlen[1], mcd2->mlen[2], mcd2->mlen[3]);
	printf ("\tbLen[] = %3d, %3d, %3d, %3d\n", mcd2->mlen[0], mcd2->mlen[1], mcd2->mlen[2], mcd2->mlen[3]);
	printf ("\tTheshold Falling = %d,  Theshold Rising = %d\n", mcd2->thresFalling, mcd2->thresRising);
}

#define __chk_input_arg__(VAL, NAME, ARG, MIN, MAX) {\
	int _t = atoi(argv[ARG]);\
	if ((_t < MIN) || (_t > MAX)) {\
		printf ("%s read: invalid <%s> (%d-%d)\n", argv[0], NAME, MIN, MAX);\
		LIN_close();\
		return (-1);\
	}\
	VAL = (unsigned char)(_t & 0x0f);\
}

int test_lin_frames (int argc, char * argv[])
{
	int i, num, usec=1000000;
	uint32_t t, s;
	
	if (argc<3) {
		printf ("Usage:\n\t%s start <m>\n", argv[0]);
		printf ("\t%s read <id> <eeprom>\n", argv[1]);
		printf ("\t%s write <id> <eeprom> <mlen0> <mlen1> <mlen2> <mlen3> <blen0> <blen1> <blen2> <blen3> <rise> <fall>\n", argv[0]);
		printf ("Options:\n");
		printf ("\t<m>: the measurement setup number for start measurement (0-3).\n");
		printf ("\t<id>: the slave ID to read/write measurement config (0-7)\n");
		printf ("\t<eeprom>: whether to read from or write to EEPROM (0=NO, 1=YES).\n");
		printf ("\t<mlenX>: the period (msec) to run echo detection in setup X (0-255)\n");
		printf ("\t<blenX>: the number of pulse to burst in setup X (0-15)\n");
		printf ("\t<rise>: the number of steps to reduce threshold (0-15).\n");
		printf ("\t<fall>: the number of steps to increase threshold (0-15).\n");
		return (0);
	}
	if (strcmp(argv[1], "start") == 0) {
		// start measurement
		int m;
		__chk_input_arg__(m, "m", 2, 0, 3);
		printf ("Sending StartMeasurement Frame ...\n");
		LIN_send_StartMeasurement(m);
		printf ("Done.\n");
	}
	if (strcmp(argv[1], "read") == 0) {
		// read measurement config
		int eeprom = 0;
		LIN_MEASURE_CFG_DATA2_t mcd2;
		if (argc < 4) {
			printf ("%s read: requires <id> and <eeprom>\n", argv[0]);
			return (-1);
		}
		__chk_input_arg__(mcd2.sID, "id", 2, 0, 7);
		__chk_input_arg__(eeprom, "eeprom", 3, 0, 1);
		printf ("Reading measurement configuration from device ...\n");
		if (LIN_send_MeasurementConfig2Read (&mcd2, eeprom) < 0) {
			printf ("Error in sending/receiving LIN frames.\n");
			return (-1);
		}
		printf ("Done\n\nReceived ");
		print_measurement_config (&mcd2);
	}
	if (strcmp(argv[1], "write") == 0) {
		// write measurement config
		int id, eeprom = 0;
		LIN_MEASURE_CFG_DATA2_t mcd2;
		if (argc < 14) {
			printf ("%s write: requires <id> <eeprom> <mlen0> <mlen1> <mlen2> <mlen3> <blen0> <blen1> <blen2> <blen3> <rise> <fall>\n", argv[0]);
			return (-1);
		}
		__chk_input_arg__(mcd2.sID, "id", 2, 0, 7);
		__chk_input_arg__(eeprom, "eeprom", 3, 0, 1);
		__chk_input_arg__(mcd2.mlen[0], "mlen0", 4, 0, 255);
		__chk_input_arg__(mcd2.mlen[1], "mlen1", 5, 0, 255);
		__chk_input_arg__(mcd2.mlen[2], "mlen2", 6, 0, 255);
		__chk_input_arg__(mcd2.mlen[3], "mlen3", 7, 0, 255);
		__chk_input_arg__(mcd2.blen[0], "blen0", 8, 0, 15);
		__chk_input_arg__(mcd2.blen[1], "blen1", 9, 0, 15);
		__chk_input_arg__(mcd2.blen[2], "blen2", 10, 0, 15);
		__chk_input_arg__(mcd2.blen[3], "blen3", 11, 0, 15);
		__chk_input_arg__(mcd2.thresRising, "rise", 10, 0, 15);
		__chk_input_arg__(mcd2.thresFalling, "fall", 11, 0, 15);
		printf ("Writing measurement configuration to device ...\n");
		LIN_send_MeasurementConfig2Write (&mcd2, eeprom);
		printf ("Done.\n");
		printf ("Reading measurement configuration from device ...\n");
		if (LIN_send_MeasurementConfig2Read (&mcd2, eeprom) < 0) {
			printf ("Error in sending/receiving LIN frames.\n");
			LIN_close();
			return (-1);
		}
		printf ("Done\n\nReceived ");
		print_measurement_config (&mcd2);
	}
	return (0);
}

void test_lin_pins (void)
{
	int brk=1, txe=0, rxe=1;
	int c;
	
	while (1) {
		printf ("\n**** BRK=%d  TX-E=%d  RX-E=%d ****\n\n", brk, txe, rxe);
		printf ("press (B) to toggle BRK, (T) to enable TX, (R) to enable RX, (Q) to quit.  ");
		c = fgetc(stdin);
		if ((c == 'B') || (c == 'b')) {
			if (brk == 0) {
				__LIN_SET_RX();	// go to receive mode
				brk = 1, txe = 0, rxe = 1;
			} else {
				__LIN_SET_BRK(); // go to break mode
				brk = 0, txe = 0, rxe = 0;
			}
		} 
		if ((c == 'R') || (c == 'r')) {
			__LIN_SET_RX();	// go to receive mode
			brk = 1, txe = 0, rxe = 1;
		}
		if ((c == 'T') || (c == 't')) {
			__LIN_SET_TX();	// go to transmit mode
			brk = 1, txe = 1, rxe = 0;
		}
		if ((c == 'Q') || (c == 'q'))
			break;
	}
}

void test_delay (void)
{
	uint32_t t[100], s;
	int i;
	
	printf ("\ninline printf timing:\n");
	for (i=0, s=0; i<100; i++) {
		printf ("%3d - %u\n", i, t[i] = bcm2835_st_read());
		if (i>0)
			s += t[i] - t[i-1];
	}
	printf("Average: %f\n", s/99.);
		
	for (i=0; i<100; i++)
		t[i] = bcm2835_st_read();
	printf ("\nno-printf timing:\n");
	for (i=0, s=0; i<100; i++) {
		printf ("%3d - %u\n", i, t[i]);
		if (i>0)
			s += t[i] - t[i-1];
	}
	printf("Average: %f\n", s/99.);

	for (i=0; i<100; i++) {
		t[i] = bcm2835_st_read();
		usleep(1);
	}
	printf ("\nusleep(1) timing:\n");
	for (i=0, s=0; i<100; i++) {
		printf ("%3d - %u\n", i, t[i]);
		if (i>0)
			s += t[i] - t[i-1];
	}
	printf("Average: %f\n", s/99.);	
}

#define __SERIAL_SEND_BREAK_SYNC() \
	__LIN_SET_BRK();\
	t = __sys_tick__();\
	bcm2835_delayMicroseconds(500);\
	for (; __sys_tick__() - t < 676; );\
	tcflush(__serial_fd, TCIOFLUSH);\
	__LIN_SET_TX();\
	for (t=__sys_tick__(); __sys_tick__() - t < 57; );\
	start = __sys_tick__();

#define __SERIAL_SEND_BYTE(B) \
	byte = B;\
	r = write (__serial_fd, &byte, 1);\
	if (r<0)\
		perror("write()");

#define __SEND_COMPLETE(NUMBYTE) \
	for (byte=1; byte > 0; )\
		ioctl(__serial_fd, TIOCOUTQ, &byte);\
	for (;__sys_tick__() - start < NUMBYTE*521;);\
	__LIN_SET_RX();

#define __SEND_START_MEASUREMENT_0() \
	__SERIAL_SEND_BYTE(0x55);\
	__SERIAL_SEND_BYTE(0x80);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0x00);\
	__SERIAL_SEND_BYTE(0x2a);\
	__SEND_COMPLETE(6);

#define __SEND_START_MEASUREMENT_1() \
	__SERIAL_SEND_BYTE(0x55);\
	__SERIAL_SEND_BYTE(0x80);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0x01);\
	__SERIAL_SEND_BYTE(0x29);\
	__SEND_COMPLETE(6);
#define __SEND_START_MEASUREMENT_2() \
	__SERIAL_SEND_BYTE(0x55);\
	__SERIAL_SEND_BYTE(0x80);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0x02);\
	__SERIAL_SEND_BYTE(0x28);\
	__SEND_COMPLETE(6);

#define __SEND_START_MEASUREMENT_3() \
	__SERIAL_SEND_BYTE(0x55);\
	__SERIAL_SEND_BYTE(0x80);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0xaa);\
	__SERIAL_SEND_BYTE(0x03);\
	__SERIAL_SEND_BYTE(0x27);\
	__SEND_COMPLETE(6);

#define __SEND_CFG_DATA_REQ_1() \
	__SERIAL_SEND_BYTE(0x55);\
	__SERIAL_SEND_BYTE(0x39);\
	__SERIAL_SEND_BYTE(0x01);\
	__SERIAL_SEND_BYTE(0xc5);\
	__SEND_COMPLETE(4);

#define __SEND_CFG_DATA_REQ_2() \
	__SERIAL_SEND_BYTE(0x55);\
	__SERIAL_SEND_BYTE(0x39);\
	__SERIAL_SEND_BYTE(0x02);\
	__SERIAL_SEND_BYTE(0xc4);\
	__SEND_COMPLETE(4);

#define __SEND_MEAS_CFG_DATA_READ() \
	__SERIAL_SEND_BYTE(0x55);\
	__SERIAL_SEND_BYTE(0xb1);\
	__SEND_COMPLETE(2);


int cont_test_send (void)
{
	unsigned char msg[] = { 0x11, 0x22, 0x44, 0x88, 0xcc, 0x66 };
	unsigned char byte = 0x55;
	int r, i;
	uint32_t t, start;
	
	printf("0x%x \n", msg[0]);
	while (1) {
		fd_set rfds;
		struct timeval tv;
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		tv.tv_sec=1;
		tv.tv_usec=0;
		r = select(1, &rfds, NULL, NULL, &tv);
		if (r>0)
			break;		
		
		printf ("sending 6 bytes\n");
		__LIN_SET_BRK();
		t = __sys_tick__();
		bcm2835_delayMicroseconds(500);
		for (; __sys_tick__() - t < 676; );
		__LIN_SET_TX();
		for (t=__sys_tick__(); __sys_tick__() - t < 57; );
		byte = 0x55;
		start = __sys_tick__();
		for (i=0; i<6; i++) {
			byte = msg[i];
			r = write (__serial_fd, &byte, 1);
			if (r<0)
				perror("write()");
		}
		for (;__sys_tick__() - start < 6*521;);
		__LIN_SET_RX();
	}
}

int cont_test_send_startmeas0 (void)
{
	unsigned char byte;
	uint32_t t, start;
	int r;

	printf ("sending StartMeasurement (Setup=0) ...\n");
	__SERIAL_SEND_BREAK_SYNC();
	__SEND_START_MEASUREMENT_0();
}

int cont_test_send_startmeas1 (void)
{
	unsigned char byte;
	uint32_t t, start;
	int r;

	printf ("sending StartMeasurement (Setup=1) ...\n");
	__SERIAL_SEND_BREAK_SYNC();
	__SEND_START_MEASUREMENT_1();
}

int cont_test_send_startmeas3 (void)
{
	unsigned char byte;
	uint32_t t, start;
	int r;

	printf ("sending StartMeasurement (Setup=3) ...\n");
	__SERIAL_SEND_BREAK_SYNC();
	__SEND_START_MEASUREMENT_3();
}

int cont_test_send_startmeas2 (void)
{
	unsigned char byte;
	uint32_t t, start;
	int r;

	printf ("sending StartMeasurement (Setup=2) ...\n");
	__SERIAL_SEND_BREAK_SYNC();
	__SEND_START_MEASUREMENT_2();
}

int cont_test_send_cfgdatareq1 (void)
{
	unsigned char byte;
	uint32_t t, start;
	int r;

	printf ("sending ConfigDataRequest (ID=3) ...\n");
	__SERIAL_SEND_BREAK_SYNC();
	__SEND_CFG_DATA_REQ_1();
}

int cont_test_send_cfgdatareq2 (void)
{
	unsigned char byte;
	uint32_t t, start;
	int r;

	printf ("sending ConfigDataRequest (ID=2) ...\n");
	__SERIAL_SEND_BREAK_SYNC();
	__SEND_CFG_DATA_REQ_2();
}

int cont_test_send_mcfgdata2r (void)
{
	unsigned char byte;
	uint32_t t, start;
	int r;

	printf ("sending MeasConfigData2Read ...\n");
	__SERIAL_SEND_BREAK_SYNC();
	__SEND_MEAS_CFG_DATA_READ();
	
	start = __sys_tick__();
	while (__sys_tick__() - start < 1000) {
		r = read(__serial_fd, &byte, 1);
		if ((r < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK))
			perror("read()");
		else if (r>0) {
			printf("*** READ: 0x%02x\n", byte);
			start = __sys_tick__();
		}
	}
}


int cont_test_send_3 (void)
{
	fd_set rfds;
	struct timeval tv;	
	int r;
	unsigned loop;
	
	for (loop=0;;loop++) {
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		tv.tv_sec=0;
		tv.tv_usec=100000;
		r = select(1, &rfds, NULL, NULL, &tv);
		if (r>0)
			break;
		printf ("the loop=%d\n", loop&0x1);
		switch (loop & 0x01) {
			case 0:
				cont_test_send_startmeas3();
				break;
			case 1:
				cont_test_send_startmeas3();
				break;
			case 2:
				cont_test_send_startmeas2();
				break;
			case 3:
				cont_test_send_startmeas3();
				break;
			/*
			case 4:
				cont_test_send_cfgdatareq2();
				break;
			case 5:
				cont_test_send_mcfgdata2r();
				break;
			*/
		}
	}
}

int cont_test_send_2 (void)
{
	unsigned char byte = 0x55;
	int r, i, byte_cnt, loop;
	uint32_t t, start;
	
	for (loop=0;;loop++) {
		fd_set rfds;
		struct timeval tv;
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		tv.tv_sec=1;
		tv.tv_usec=0;
		r = select(1, &rfds, NULL, NULL, &tv);
		if (r>0)
			break;
		switch (loop & 0x7) {
			case 0:
				printf ("sending StartMeasurement (setup=0) ...\n");
				__SERIAL_SEND_BREAK_SYNC();
				__SEND_START_MEASUREMENT_0();
				break;
			case 1:
				printf ("sending StartMeasurement (setup=1) ...\n");
				__SERIAL_SEND_BREAK_SYNC();
				__SEND_START_MEASUREMENT_1();
				break;
			
			case 2:
				printf ("sending ConfigDataRequest (ID=1) ...\n");
				__SERIAL_SEND_BREAK_SYNC();
				__SEND_CFG_DATA_REQ_1();
				break;
			case 3:
				printf ("sending MeasurementConfigData2Read ...\n");
				__SERIAL_SEND_BREAK_SYNC();
				__SEND_MEAS_CFG_DATA_READ();
				break;
			case 4:
				printf ("sending ConfigDataRequest (ID=2) ...\n");
				__SERIAL_SEND_BREAK_SYNC();
				__SEND_CFG_DATA_REQ_2();
				break;
			case 5:
				printf ("sending MeasurementConfigData2Read ...\n");
				__SERIAL_SEND_BREAK_SYNC();
				__SEND_MEAS_CFG_DATA_READ();
				break;
		}
	}
}

int cont_test_send_4 (void)
{
	fd_set rfds;
	struct timeval tv;	
	int i = 0, r;
	
	for (i=0;;i++) {
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		tv.tv_sec=0;
		tv.tv_usec=800000;
		r = select(1, &rfds, NULL, NULL, &tv);
		if (r>0)
			break;
		switch (i & 0x1) {
			case 1:
				cont_test_send_cfgdatareq1();
				break;
			case 0:
				cont_test_send_mcfgdata2r();
				break;
		}
	}
}
		
				
int main (int argc, char * argv[])
{
	if (bcm2835_init() != 1) {
		printf ("bcm2835_init() failed.\n");
		return (-1);
	}

	LIN_init();
	
	cont_test_send_3();	
	
	LIN_close();
	
	return (0);
}

