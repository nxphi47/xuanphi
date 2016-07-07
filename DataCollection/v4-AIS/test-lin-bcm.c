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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
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

#define LIN_PIN_TX_DBG	12			// Debug probe for TX
#define LIN_PIN_RX_DBG	13			// Debug probe for RX

#define __LIN_SET_RX()	\
	bcm2835_gpio_clr(LIN_PIN_TX_EN); \
	bcm2835_gpio_set_multi((1<<LIN_PIN_RX_EN)|(1<<LIN_PIN_BREAK))
	
#define __LIN_SET_TX()	\
	bcm2835_gpio_clr(LIN_PIN_RX_EN); \
	bcm2835_gpio_set_multi((1<<LIN_PIN_TX_EN)|(1<<LIN_PIN_BREAK))

#define __LIN_SET_BRK()	\
	bcm2835_gpio_clr_multi((1<<LIN_PIN_TX_EN) | (1<<LIN_PIN_RX_EN) | (1<<LIN_PIN_BREAK))

static int __serial_fd = -1;
static struct termios __serial_org_termios;

/* Initialize the LIN interface
 */
int LIN_init (void)
{
	struct termios s;
	
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
	
	tcsetattr(__serial_fd, TCSANOW, &s);
	tcflush(__serial_fd, TCIOFLUSH);
	
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

// RX/TX Debug Code
#define DBG_SIZE 	65536
typedef struct __tag_lin_dbg_probe__ {
	uint32_t tstart;
	int curr;
	uint32_t tofs[DBG_SIZE];
	unsigned char tx[DBG_SIZE];
	unsigned char rx[DBG_SIZE];
	char usr[DBG_SIZE];
} LIN_DBG_PROBE_t, * LIN_DBG_PROBE_p;
	
static LIN_DBG_PROBE_p __dbgp = NULL;
static FILE * __dbgfp = NULL;

static void LIN_DBG_reset (void)
{
	if (__dbgp) {
		memset (__dbgp->tx, 0x3f, DBG_SIZE);
		__dbgp->curr = -1;
	}
}

static void LIN_DBG_init (void)
{
	__dbgp = (LIN_DBG_PROBE_p)calloc(1, sizeof(LIN_DBG_PROBE_t));
    
	bcm2835_gpio_fsel(LIN_PIN_TX_DBG, BCM2835_GPIO_FSEL_INPT); /* TX-Debug */
	bcm2835_gpio_set_pud(LIN_PIN_TX_DBG, BCM2835_GPIO_PUD_UP);
	bcm2835_gpio_fsel(LIN_PIN_RX_DBG, BCM2835_GPIO_FSEL_INPT); /* RX-Debug */
	bcm2835_gpio_set_pud(LIN_PIN_RX_DBG, BCM2835_GPIO_PUD_UP);
	LIN_DBG_reset();
	
	__dbgfp = fopen("lin-debug-probe.txt", "a");
}

static void LIN_DBG_close (void)
{
	if (__dbgp) {
		free (__dbgp);
		__dbgp = NULL;
		if (__dbgfp) 
			fclose(__dbgfp);
		__dbgfp = NULL;
	}
}

// number of probes (each probe takes 10usec!!!!)
static void LIN_DBG_probe (char c, int n)
{
	int i;
	
	if (__dbgp && (n>0)) {
		if (__dbgp->curr == -1) {
			__dbgp->tstart = bcm2835_st_read();
			__dbgp->curr = 0;
		}
		for (i=0; i<n; i++) {
			__dbgp->tofs[__dbgp->curr] = bcm2835_st_read();
			__dbgp->tx[__dbgp->curr] = bcm2835_gpio_lev(LIN_PIN_TX_DBG);
			__dbgp->rx[__dbgp->curr] = bcm2835_gpio_lev(LIN_PIN_RX_DBG);
			__dbgp->usr[__dbgp->curr] = c;
			__dbgp->curr ++;
			if (__dbgp->curr >= DBG_SIZE)
				__dbgp->curr = 0;
			if (i >= (n-1))
				break;
			bcm2835_delayMicroseconds(10);
		}
	}
}

static void LIN_DBG_dump (void)
{
	if (__dbgp && __dbgfp) {
		int s;
		if ((s = __dbgp->curr) < 0)
			return;
		if (__dbgp->tx[s] == 0x3f)
			s = 0;
		fprintf (__dbgfp, "-- Probe Start Time: %10u\n", __dbgp->tstart);
		for (; s!= __dbgp->curr; s++)
			fprintf (__dbgfp, "-- %10.3f  %c  TX=%c  RX=%c\n", 
				(__dbgp->tofs[s]-__dbgp->tstart)/1000.0, 
				__dbgp->usr[s], __dbgp->tx[s]?'1':'0', __dbgp->rx[s]?'1':'0');
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
	LIN_DBG_probe('b',1);
	t = bcm2835_st_read();
	__LIN_SET_BRK();
	tcflush(__serial_fd, TCIFLUSH);
	LIN_DBG_probe('B',1);

	// do calculations here
	pid = __lin_pid_lut__[id&0x3F];
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	for (byte=0; byte<len; byte++) 
		chksum += data[byte];
	chksum = ~(chksum + (chksum >> 8)) & 0xFF;
	LIN_DBG_probe('c',1);

#ifdef __DEBUG__
	sprintf (debugstr, "0x%02x ", pid);
	for (byte=0; byte<len; byte++) 
		sprintf (debugstr + (byte+1)*5, "0x%02x ", data[byte]);
	sprintf (debugstr + (byte+1)*5, "0x%02x ", chksum);	
#endif

	// wait for break to finish
	// LIN requires 13 bit of 0 (Dorminant) to break the signal.
	// At 19.2kHz, that is 677.08usec of 0-bits
	LIN_DBG_probe('C',10);
	bcm2835_delayMicroseconds(t + 677 - bcm2835_st_read());
	LIN_DBG_probe('f',1);
	__LIN_SET_TX();
	// one bit of recesive as break delimiter
	LIN_DBG_probe('d',1);
	bcm2835_delayMicroseconds(52);
	LIN_DBG_probe('D',1);
	
	// send sync field and the frame and then chksum
	debug("-- LIN_send_frame(): sending frame [ %s ] ...\n", debugstr);
	r = write (__serial_fd, &sync, 1);
	if (r<0)
		perror("write\n");
	LIN_DBG_probe('u',1);
	r = write (__serial_fd, &pid, 1);
	if (r<0)
		perror("write\n");
	LIN_DBG_probe('v',1);
	for (byte=0; byte<len; ) {
		byte += write(__serial_fd, data+byte, 1);
		LIN_DBG_probe('V',1);
	}
	write(__serial_fd, &chksum, 1);

	// wait for TX queue to be empty
	debug("-- LIN_send_frame(): waiting for transmission to complete ...\n");
	for (byte=1; byte > 0; ) {
		LIN_DBG_probe('T',1);
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
	LIN_DBG_probe('r',1);
	__LIN_SET_RX();	
	LIN_DBG_probe('R',1);
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
	if (__dbgfp)
		fprintf (__dbgfp, "\n%10u: ***** Sending Start Frame ****\n", bcm2835_st_read());
	LIN_send_frame (LIN_ELMOS_START_MEASUREMENT, 3, frame);
	LIN_DBG_dump();
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


void test_lin_frames_2 (void)
{
	LIN_MEASURE_CFG_DATA2_t mcd2;
	int c, i;
	
	while (1) {
		printf ("\nPress [S] to send StartMeasurement\n");
		printf ("Press [R] to send ReadMeasurementConfig\n");
		printf ("Press [Q] to quit\n");
		c = fgetc(stdin);
		for (i=0; i<10; i++) {
		switch (c) {
		case 'S': case 's':
			printf ("Sending StartMeasurement ....\n");
			LIN_send_StartMeasurement(2);
			break;
		case 'R': case 'r':	
			printf ("Sending MeasurementConfigRead to 0x01 ....\n");
			mcd2.sID = 0x01;
			if (LIN_send_MeasurementConfig2Read(&mcd2, 0) >= 0) {
				print_measurement_config(&mcd2);
				break;
			}
			printf ("Sending MeasurementConfigRead to 0x02 ....\n");
			mcd2.sID = 0x02;
			if (LIN_send_MeasurementConfig2Read(&mcd2, 0) >= 0) {
				print_measurement_config(&mcd2);
				break;
			}
			printf ("Sending MeasurementConfigRead to 0x0f ....\n");
			mcd2.sID = 0x0f;
			if (LIN_send_MeasurementConfig2Read(&mcd2, 0) >= 0) {
				print_measurement_config(&mcd2);
				break;
			}
			break;
		case 'Q': case 'q':
			return;
		default:
			break;
		}
		usleep(200000);
		}
		while (c!='\n')
			c = fgetc(stdin);
	}
}
				
int main (int argc, char * argv[])
{
	if (bcm2835_init() != 1) {
		printf ("bcm2835_init() failed.\n");
		return (-1);
	}

	LIN_init();
		
	//LIN_DBG_init();
	
	//test_delay();
	//test_lin_pins();
	//test_lin_frames(argc, argv);
	test_lin_frames_2();
	
	//LIN_DBG_close();
	
	LIN_close();
	
	return (0);
}

