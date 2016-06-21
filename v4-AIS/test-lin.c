/*
	Convenient APIs for Data Collection Circuit
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// for serial interface
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif

#define __DISABLE_INTERRUPT__

/* --------------- from minimal_gpio.c -------------------
 * http://abyz.co.uk/rpi/pigpio/examples.html
 * ------------------------------------------------------- */

/* definition of modes are: */
#define RPIGPIO_INPUT  0
#define RPIGPIO_OUTPUT 1
#define RPIGPIO_ALT0   4
#define RPIGPIO_ALT1   5
#define RPIGPIO_ALT2   6
#define RPIGPIO_ALT3   7
#define RPIGPIO_ALT4   3
#define RPIGPIO_ALT5   2

/* definition of pull-up, pull-down are: */
#define RPIGPIO_PUD_OFF  0
#define RPIGPIO_PUD_DOWN 1
#define RPIGPIO_PUD_UP   2


#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

static volatile uint32_t __piModel = 1;
static volatile uint32_t __piPeriphBase = 0x20000000;
static volatile uint32_t __piBusAddr = 0x40000000;

#define SYST_BASE	(__piPeriphBase + 0x003000)
#define DMA_BASE 	(__piPeriphBase + 0x007000)
#define CLK_BASE 	(__piPeriphBase + 0x101000)
#define GPIO_BASE	(__piPeriphBase + 0x200000)
#define UART0_BASE	(__piPeriphBase + 0x201000)
#define PCM_BASE	(__piPeriphBase + 0x203000)
#define SPI0_BASE	(__piPeriphBase + 0x204000)
#define I2C0_BASE	(__piPeriphBase + 0x205000)
#define PWM_BASE	(__piPeriphBase + 0x20C000)
#define UART1_BASE	(__piPeriphBase + 0x215000)
#define I2C1_BASE	(__piPeriphBase + 0x804000)
#define I2C2_BASE	(__piPeriphBase + 0x805000)
#define DMA15_BASE	(__piPeriphBase + 0xE05000)
#define INTR_BASE 	(__piPeriphBase + 0x00B000)

#define DMA_LEN		0x1000 /* allow access to all channels */
#define CLK_LEN 	0xA8
#define GPIO_LEN	0xB4
#define SYST_LEN	0x1C
#define PCM_LEN 	0x24
#define PWM_LEN 	0x28
#define I2C_LEN 	0x1C
#define INTR_LEN	0x100

#define GPSET0 7
#define GPSET1 8

#define GPCLR0 10
#define GPCLR1 11

#define GPLEV0 13
#define GPLEV1 14

#define GPPUD     37
#define GPPUDCLK0 38
#define GPPUDCLK1 39

#define SYST_CS  0
#define SYST_CLO 1
#define SYST_CHI 2

static volatile uint32_t  *__gpioReg = MAP_FAILED;
static volatile uint32_t  *__systReg = MAP_FAILED;
static volatile uint32_t  *__gpioRead;
static volatile uint32_t  *__gpioSet;
static volatile uint32_t  *__gpioClr;
#ifdef __DISABLE_INTERRUPT__
static volatile uint32_t  *__intrReg = MAP_FAILED;
#endif

#define PI_BANK (gpio>>5)
#define PI_BIT  (1<<(gpio&0x1F))

/* gpio modes. * 

#define PI_INPUT  0
#define PI_OUTPUT 1
#define PI_ALT0   4
#define PI_ALT1   5
#define PI_ALT2   6
#define PI_ALT3   7
#define PI_ALT4   3
#define PI_ALT5   2

*/

static void gpioSetMode(unsigned gpio, unsigned mode)
{
	int reg = gpio/10, shift = (gpio%10) * 3;
	__gpioReg[reg] = (__gpioReg[reg] & ~(7<<shift)) | (mode<<shift);
}

static int gpioGetMode(unsigned gpio)
{
	int reg = gpio/10, shift = (gpio%10) * 3;
	return (*(__gpioReg + reg) >> shift) & 7;
}

/* Values for pull-ups/downs off, pull-down and pull-up. 
already defined in rpi-gpio.h
#define PI_PUD_OFF  0
#define PI_PUD_DOWN 1
#define PI_PUD_UP   2
*/
static void gpioSetPullUpDown(unsigned gpio, unsigned pud)
{
	*(__gpioReg + GPPUD) = pud;
	usleep(20);
	*(__gpioReg + GPPUDCLK0 + PI_BANK) = PI_BIT;
	usleep(20);
	*(__gpioReg + GPPUD) = 0;
	*(__gpioReg + GPPUDCLK0 + PI_BANK) = 0;
}

static inline int gpioRead(unsigned gpio)
{
	if ((*(__gpioReg + GPLEV0 + PI_BANK) & PI_BIT) != 0) return 1;
	else                                         return 0;
}

static inline void gpioWrite(unsigned gpio, unsigned level)
{
   if (level == 0) *(__gpioReg + GPCLR0 + PI_BANK) = PI_BIT;
   else            *(__gpioReg + GPSET0 + PI_BANK) = PI_BIT;
}

/* Bit (1<<x) will be set if gpio x is high. */

static inline uint32_t gpioReadBank1(void) { return (*__gpioRead); }
static inline uint32_t gpioReadBank2(void) { return (*(__gpioRead+1)); }

/* To clear gpio x bit or in (1<<x). */

static inline void gpioClearBank1(uint32_t bits) { *__gpioClr = bits; }
static inline void gpioClearBank2(uint32_t bits) { *(__gpioClr+1) = bits; }

/* To set gpio x bit or in (1<<x). */

static inline void gpioSetBank1(uint32_t bits) { *__gpioSet = bits; }
static inline void gpioSetBank2(uint32_t bits) { *(__gpioSet+1) = bits; }

static unsigned gpioHardwareRevision(void)
{
	static unsigned rev = 0;
	FILE * filp;
	char buf[512];
	char term;
	int chars=4; /* number of chars in revision string */

	if (rev) return rev;
	__piModel = 0;
	filp = fopen ("/proc/cpuinfo", "r");
	if (filp != NULL) {
		while (fgets(buf, sizeof(buf), filp) != NULL) {
			if (__piModel == 0) {
				if (!strncasecmp("model name", buf, 10)) {
					if (strstr (buf, "ARMv6") != NULL) {
						__piModel = 1;
						chars = 4;
						__piPeriphBase = 0x20000000;
						__piBusAddr = 0x40000000;
					}
					else if (strstr (buf, "ARMv7") != NULL) {
						__piModel = 2;
						chars = 6;
						__piPeriphBase = 0x3F000000;
						__piBusAddr = 0xC0000000;
					}
				}
			}
			if (!strncasecmp("revision", buf, 8)) {
				if (sscanf(buf+strlen(buf)-(chars+1), "%x%c", &rev, &term) == 2) {
					if (term != '\n') 
						rev = 0;
				}
			}
		}
		fclose(filp);
	}
	return rev;
}

/* Returns the number of microseconds after system boot. Wraps around
   after 1 hour 11 minutes 35 seconds.
*/
static inline uint32_t gpioTick(void) { return __systReg[SYST_CLO]; }

#ifdef __DISABLE_INTERRUPT__
// enable (1) or disable (0) interrupt
static int __gpio_interrupt (int flag) 
{
	static uint32_t sav132 = 0;
	static uint32_t sav133 = 0;
	static uint32_t sav134 = 0;

	if (flag == 0) {
		// disable
		if (sav132 != 0) {
			// Interrupts already disabled so avoid printf
			return (-1);
		}
		if ((*(__intrReg+128) | *(__intrReg+129) | *(__intrReg+130)) != 0) {
			return (-2);
		}
		sav134 = *(__intrReg+134);
		*(__intrReg+137) = sav134;
		sav132 = *(__intrReg+132);  // save current interrupts
		*(__intrReg+135) = sav132;  // disable active interrupts
		sav133 = *(__intrReg+133);
		*(__intrReg+136) = sav133;
	} else {
		// flag = 1 enable
		if (sav132 == 0) {
			// Interrupts not disabled
			return (-1);
		}
		*(__intrReg+132) = sav132;    // restore saved interrupts
		*(__intrReg+133) = sav133;
		*(__intrReg+134) = sav134;
		sav132 = 0;                 // indicates interrupts enabled
	}
	return (0);
}

#define	rpigpio_enable_interrupt()	__gpio_interrupt(1)
#define	rpigpio_disable_interrupt()	__gpio_interrupt(0)

#else

#define	rpigpio_enable_interrupt()
#define	rpigpio_disable_interrupt()

#endif


/* Map in registers. */
static inline uint32_t * initMapMem (int fd, uint32_t addr, uint32_t len)
{
	return (uint32_t *) mmap(0, len, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED|MAP_LOCKED, fd, addr);
}

static int gpioInitialise(void)
{
	int fd;

	gpioHardwareRevision(); /* sets piModel, needed for peripherals address */
	fd = open("/dev/mem", O_RDWR | O_SYNC) ;
	if (fd < 0) {
		fprintf(stderr, "This program needs root privileges.  Try using sudo\n");
		return -1;
	}
	__gpioReg  = initMapMem(fd, GPIO_BASE,  GPIO_LEN);
	__systReg  = initMapMem(fd, SYST_BASE,  SYST_LEN);
#ifdef __DISABLE_INTERRUPT__
	__intrReg  = initMapMem(fd, INTR_BASE,  INTR_LEN);
#endif

	close(fd);

	if ((__gpioReg == MAP_FAILED) || (__systReg == MAP_FAILED)
#ifdef __DISABLE_INTERRUPT__
		|| (__intrReg == MAP_FAILED)
#endif
	) {
		fprintf(stderr, "Bad, mmap failed\n");
		return -1;
	}
	__gpioRead = __gpioReg + GPLEV0;
	__gpioSet = __gpioReg + GPSET0;
	__gpioClr = __gpioReg + GPCLR0;
	
	return 0;
}

// ------------------------- end of minimal_gpio.c ---------------------

// sleep for the requested number of usec
// returns the current usec count
uint32_t rpi_usleep(int n) 
{
	uint32_t st = gpioTick();
	if (n > 300) {
		// using usleep will incur an average of 140usec context switch time
		// hence, we only call usleep if the requested sleep time is more than 300us
		usleep(n-150);
	}
	for (; gpioTick() - st < n; );
	return (gpioTick());
}



/* -------------- Serial interface ------------------- */

/* You need to disable the console login of serial port with 'sudo raspi-config'.
 * Piv2 uses /dev/tty/AMA0
 * Piv3 uses /dev/tty/S0
 */
static char __serial_dev[] = "/dev/tty/S0";
static int __serial_fd = -1;
static struct termios __serial_org_termios;

/* we are using Pin 11 (GPIO17) for direction control 
 * and Pin 12 (GPIO18) for break sending
 */
#define RPIGPIO_LINCTRL		17
#define RPIGPIO_BREAK		18
#define RPIGPIO_TX0			14
#define RPIGPIO_RX0			15

#define __CONFIGURE_GPIO(PIN,INOUT,PUPDN) \
	gpioSetMode(PIN,INOUT);	\
	gpioSetPullUpDown(PIN,PUPDN)

#define __SERIAL_SET_RX()	\
	gpioWrite(RPIGPIO_BREAK, 1); \
	gpioWrite(RPIGPIO_LINCTRL, 1)
	
#define __SERIAL_SET_TX()	\
	gpioWrite(RPIGPIO_BREAK, 1); \
	gpioWrite(RPIGPIO_LINCTRL, 0)

#define __SERIAL_SET_BRK()	\
	gpioWrite(RPIGPIO_BREAK, 0); 

/* Initialize the LIN interface
 */
int LIN_init (void)
{
	struct termios s;
	
	if (__serial_fd > 0)	// already initialized
		return (0);
	
	// configure the GPIO
	gpioSetMode (RPIGPIO_TX0, RPIGPIO_ALT0);
	gpioSetMode (RPIGPIO_RX0, RPIGPIO_ALT0);
	__CONFIGURE_GPIO(RPIGPIO_LINCTRL, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
	__CONFIGURE_GPIO(RPIGPIO_BREAK, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
	
	// set default signal
	__SERIAL_SET_RX();
	
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

	// setspeed
	cfsetispeed(&s, B19200);
	cfsetospeed(&s, B19200);
	
	// set up new termios
	s.c_iflag = IGNBRK | IGNPAR | IGNCR;
	s.c_oflag = 0;
	s.c_cflag = CS8 | CLOCAL | CREAD;
	s.c_lflag = 0;
	s.c_cc[VMIN] = 0;
	s.c_cc[VTIME] = 100;
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
		__CONFIGURE_GPIO(RPIGPIO_LINCTRL, RPIGPIO_INPUT, RPIGPIO_PUD_UP);
		__CONFIGURE_GPIO(RPIGPIO_BREAK, RPIGPIO_INPUT, RPIGPIO_PUD_UP);
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

static void __lin_send_break_sync__ (void)
{
	unsigned char c = 0x55;
	
	__SERIAL_SET_BRK();
}

// Transmit a frame
// 	<id> is the contains only the first 6 bits: the P0 and P1 protection will be calculated by this function
//	<len> is the number of data bytes
//	<data> is the array of bytes to send
//	Checksum will be calculated by this function
void LIN_send_frame (unsigned char id, int len, unsigned char * data )
{
	unsigned char pid, sync=0x55;
	int byte;
	uint32_t chksum, t;

	// break the line
	t = gpioTick();
	__SERIAL_SET_BRK();

	// do calculations here
	pid = __lin_pid_lut__[id&0x3F];
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	for (byte=0; byte<len; byte++) 
		chksum += data[byte];
	chksum = ~(chksum + (chksum >> 8)) & 0xFF;

	// wait for break to finish
	// LIN requires 13 bit of 0 (Dorminant) to break the signal.
	// At 19.2kHz, that is 677.08usec of 0-bits
	rpi_usleep(t + 677 - gpioTick());
	__SERIAL_SET_TX();
	
	// send sync field and the frame and then chksum
	write (__serial_fd, &sync, 1);
	write (__serial_fd, &pid, 1);
	for (byte=0; byte<len; )
		byte += write(__serial_fd, data+byte, 1);
	write(__serial_fd, &chksum, 1);

	// wait for TX queue to be empty
	for (byte=1; byte > 0; )
		ioctl(__serial_fd, TIOCOUTQ, &byte);

	// clear any receive data
	tcflush(__serial_fd, TCIFLUSH);
	
	// return the LIN bus back to idle state (recessive = 1)
	__SERIAL_SET_RX();	
}


// Poll for a response frame
// 	<id> is the contains only the first 6 bits: the P0 and P1 protection will be calculated by this function
//	<len> is the number of data bytes expected
//	<data> is the array of bytes to received the data
// NOTE: <data> must be at least <len>+1 size.  The +1 is for receiving the checksum
// Function returns 0 on success, negative value on error
int LIN_poll_frame (unsigned char id, int len, unsigned char * data)
{
	unsigned char pid, sync=0x55;
	int byte;
	uint32_t chksum, start;
	
	// break the line
	start = gpioTick();
	__SERIAL_SET_BRK();

	// do calculations here
	pid = __lin_pid_lut__[id&0x3F];
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	
	// wait for break to finish
	// LIN requires 13 bit of 0 (Dorminant) to break the signal.
	// At 19.2kHz, that is 677.08usec of 0-bits
	rpi_usleep(start + 677 - gpioTick());
	__SERIAL_SET_TX();

	// send sync field and the frame and then chksum
	write (__serial_fd, &sync, 1);
	write (__serial_fd, &pid, 1);

	// wait for TX queue to be empty
	tcflush(__serial_fd, TCIFLUSH);
	for (byte=1; byte > 0; )
		ioctl(__serial_fd, TIOCOUTQ, &byte);

	// receive response
	__SERIAL_SET_RX();	
	start = gpioTick();
	for (byte=0; byte<=len; ) {
		byte += write(__serial_fd, data+byte, 1);
		if (gpioTick() > start + 50000) // more than 50ms 
			return (-1);
	}
	// verify checksum
	for (byte=0; byte<len; byte++)
		chksum += data[byte];
	chksum = (chksum + (chksum >> 8)) & 0xFF;
	if (chksum != 0xFF)
		return (-1);

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
	rpi_usleep(Wait)

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
	rpi_usleep(1000)

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
		rpi_usleep(20000);
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

int main (int argc, char * argv[])
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
	if (gpioInitialise() < 0)
		return (-1);
	if (LIN_init() < 0)
		return (-1);
		
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
			LIN_close();
			return (-1);
		}
		__chk_input_arg__(mcd2.sID, "id", 2, 0, 7);
		__chk_input_arg__(eeprom, "eeprom", 3, 0, 1);
		printf ("Reading measurement configuration from device ...\n");
		if (LIN_send_MeasurementConfig2Read (&mcd2, eeprom) < 0) {
			printf ("Error in sending/receiving LIN frames.\n");
			LIN_close();
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
			LIN_close();
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
	LIN_close();
	return (0);
}

