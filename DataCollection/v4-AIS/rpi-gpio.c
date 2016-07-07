/*
	Convenient APIs for Data Collection Circuit
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "rpi-gpio.h"

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

/* gpio modes. 
already defined in rpi-gpio.h

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

#ifdef __VER_4_AIS_CIRCUIT__
/* using version 4.0-AIS circuit */

/* Data Pins  to  GPIO# mapping */
#define RPIGPIO_D0		20
#define RPIGPIO_D1		21
#define RPIGPIO_D2		22
#define RPIGPIO_D3		23
#define RPIGPIO_D4		24
#define RPIGPIO_D5		25
#define RPIGPIO_D6		26
#define RPIGPIO_D7		27

/* data bits in GPIO Registers */
#define RPIGPIO_D0_bit		0x00100000	// 1 << 20
#define RPIGPIO_D1_bit		0x00200000	// 1 << 21
#define RPIGPIO_D2_bit		0x00400000	// 1 << 22
#define RPIGPIO_D3_bit		0x00800000	// 1 << 23
#define RPIGPIO_D4_bit		0x01000000	// 1 << 24
#define RPIGPIO_D5_bit		0x02000000	// 1 << 25
#define RPIGPIO_D6_bit		0x04000000	// 1 << 26
#define RPIGPIO_D7_bit		0x08000000	// 1 << 27

/* data bits shift to get data */
// because D0-D7 are mapped to continuous pins in GPIO,
// the shift is all the same
#define RPIGPIO_DATA_bits	0x0FF00000
#define RPIGPIO_DATA_shift	20


/* ADC Control Pins  to  GPIO# mapping */
#ifdef __VER_4_AIS_USE_ADC976__
/* Using 16-bits ADC976 */
#define RPIGPIO_ADC_CS		16
#define RPIGPIO_ADC_BUSY	17
#define RPIGPIO_ADC_RC		18
#define RPIGPIO_ADC_BYTE	19

#define RPIGPIO_ADC_CS_bit		0x00010000	// 1 << 16
#define RPIGPIO_ADC_BUSY_bit	0x00020000	// 1 << 17
#define RPIGPIO_ADC_RC_bit		0x00040000 	// 1 << 18 
#define RPIGPIO_ADC_BYTE_bit	0x00080000	// 1 << 19

#else
/* Using 8-bits ADC7819 */
#define RPIGPIO_ADC_CS		16
#define RPIGPIO_ADC_BUSY	17
#define RPIGPIO_ADC_RD		18
#define RPIGPIO_ADC_CONVST	19

#define RPIGPIO_ADC_CS_bit		0x00010000	// 1 << 16
#define RPIGPIO_ADC_BUSY_bit	0x00020000	// 1 << 17
#define RPIGPIO_ADC_RD_bit		0x00040000 	// 1 << 18 
#define RPIGPIO_ADC_CONVST_bit	0x00080000	// 1 << 19

#endif // ADC976 or ADC7819

/* LIN Control pins   to   GPIO# mappings */
#define RPIGPIO_LIN_RW		7
#define RPIGPIO_LIN_RX		5
#define RPIGPIO_LIN_TX		6

#define RPIGPIO_LIN_RW_bit	0x00000080	// 1 << 7
#define RPIGPIO_LIN_RX_bit	0x00000020	// 1 << 5
#define RPIGPIO_LIN_TX_bit	0x00000040	// 1 << 6

#define RPIGPIO_LIN_RW_TX_bit	0x000000c0

#endif // __VER_4_AIS_CIRCUIT__

// ------------- LIN COMMUNICATIONS ------------------------------

// RW Pin: H = RPI is transmitting; L = RPI is receiving

// send a 1 (Recessive) bit on LIN bus 
#define __LIN_TX_BIT_1__	\
	gpioSetBank1(RPIGPIO_LIN_RW_TX_bit) 

// send a 0 (Dominant) bit on LIN bus 
#define __LIN_TX_BIT_0__	\
	gpioClearBank1(RPIGPIO_LIN_TX_bit);	\ 
	gpioSetBank1(RPIGPIO_LIN_RW_bit)

// set LIN bus to receive state
#define __LIN_RX_STATE__	\
	gpioSetBank1(RPIGPIO_LIN_RW_TX_bit); \ 
	gpioClearBank1(RPIGPIO_LIN_RW_bit)

// remember the last clock tick
static uint32_t	__lin_last_tick__ = gpioTick();

// LIN Bus is 19.2kHz ==> each bit should be 52.08333usec
// The following gives the timing required for every 36 bits
static uint32_t __lin_tick_map__[] = {
	  52,  104,  156,  208,  260,  312,  365,  417,  469,  521,
	 573,  625,  677,  729,  833,  885,  937,  990, 1042, 1094,
	1146, 1198, 1250, 1302, 1354, 1406, 1458, 1510, 1562, 1615,
	1667, 1719, 1771, 1823, 1875 };
static int __lin_tick_map_ptr__ = -1; 

// time error tolerance: approximately half of a LIN bit time
#define __lin_time_tolerance__		25
#define __lin_tolerated_bit_time__	(52 + __lin_time_tolerance__)

// flag to remember if bit timing has been exceeded
static int __lin_time_exceeded__ = 0;

// wait to send next bit 
static void __lin_wait_next_tick__(void)
{
	if (__lin_tick_map_ptr__ < 0) {
		// initial transmission, don't need to wait
		__lin_last_tick__ = gpioTick();
		__lin_tick_map_ptr__ = 0;
		return;
	}
	// check for time expired error
	if (gpioTick() - __lin_last_tick__ > __lin_tick_map__[__lin_tick_map_ptr__] + __lin_time_tolerance__)
		__lin_time_exceeded__ ++;
	else
		// spin until time (almost) elapsed
		for (; gpioTick() - __lin_last_tick__ < __lin_tick_map__[__lin_tick_map_ptr__]-1;);
	if (++__lin_tick_map_ptr__ >= 35) {
		// restart from 0 after 36
		__lin_last_tick__ = gpioTick();
		__lin_tick_map_ptr__ = 0;
	}
}

// send break field: bit 0 for 13*52.083us = 677usec
// this will disable secondary interrupt as well, hence 
// no printf or other system call until interrupt is enabled again
#define __lin_send_break_field__()	\
	__lin_last_tick__ = gpioTick();	\
	__LIN_TX_BIT_0__; \
	usleep(600); \
	rpigpio_disable_interrupt(); \
	for (; gpioTick() - __lin_last_tick__ < 676;); \
	__lin_time_exceeded__ = 0; \
	__lin_tick_map_ptr__ = -1; \
	__lin_wait_next_tick__(); \
	__LIN_TX_BIT_1__

// send a byte: 8 bit, with start bit and stop bit
static void __lin_send_byte__(uint32_t byte)
{
	int bit;
	
	// start bit
	__lin_wait_next_tick__();
	__LIN_TX_BIT_0__;
	// the byte
	for (bit=0; bit<8; bit++) {
		__lin_wait_next_tick__();
		if (byte & 1) {
			__LIN_TX_BIT_1__;
		} else {
			__LIN_TX_BIT_0__;
		}
		byte >>= 1;
	}
	// stop bit
	__lin_wait_next_tick__();
	__LIN_TX_BIT_1__;
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
// Function returns number of time the bit-timing is exceeded
int rpigpio_lin_send_frame (unsigned char id, int len, unsigned char * data )
{
	unsigned char pid = __lin_pid_lut__[id&0x3F];
	int byte;
	uint32_t chksum;
	
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	
	// start sending
	__lin_send_break_field__();
	__lin_send_byte__(0x55);		// SYNC field
	__lin_send_byte__(pid);			// PID field
	for (byte=0; byte<len; byte++) {
		__lin_send_byte__(data[byte]);
		chksum += data[byte];
	}
	chksum = ~(chksum + (chksum >> 8)) & 0xFF;
	__lin_send_byte__(chksum);
	// return the LIN bus back to idle state (recessive = 1)
	__LIN_TX_BIT_1__;
	rpigpio_enable_interrupt();
	
	return (__lin_time_exceeded__);
}

static uint32_t __lin_last_rx_tick__ = 0;

// check if the LIN bus has changed state
static inline int __lin_rx_level_change__(uint32_t curr)
{
	uint32_t __tick = gpioTick();
	int i;
	
	curr = curr ? RPIGPIO_LIN_RX_bit : 0;
	for (i=0; i<3; i++) {
		for (_tick = gpioTick(); gpioTick() == _tick;);
		if (gpioReadBank1() & RPIGPIO_LIN_RX_bit == curr)
			return (0);
	}
	return (1)
}

#define RPIGPIO_RX_ERR_START_BIT	-1
#define RPIGPIO_RX_ERR_TIMING		-2
#define RPIGPIO_RX_ERR_CHKSUM		-4
#define RPIGPIO_RX_ERR_HEADER		-8
#define RPIGPIO_RX_ERR_STOP_BIT		-16
static int __lin_flag_rx_error__ = 0;

// Wait for start bit
static inline int __lin_rx_start_bit__(void)
{
	uint32_t t = gpioTick();
	
	// set to RX state
	__LIN_RX_STATE__;
	// wait for LIN bus to go into dorminant state
	while (!__lin_rx_level_change__(1)) {
		__lin_last_rx_tick__ = gpioTick();
		if (__lin_last_rx_tick__ - t > 260) {
			// did not receive start-bit for more than 5 bit time
			__lin_flag_rx_error__ = RPIGPIO_RX_ERR_START_BIT;
			return (-1);
		}
	}
	return (0);
}

// Receive a byte of data
static inline int __lin_rx_byte__ (unsigned char * rxbyte)
{
	int bit_cnt = -1;		// first bit is stop bit, ignored
	uint32_t tick = 0;
	uint32_t byte = 0;
	uint32_t bit = 0;
	
	__lin_flag_rx_error__ = 0;
	if (__lin_rx_start_bit__() < 0)
		return (__lin_flag_rx_error__);
	while (bit_cnt < 8) {
		t_elapsed = (tick = gpioTick()) - __lin_last_rx_tick__;
		if (t_elapsed > __lin_tolerated_bit_time__) {
			// timing error: we missed an entire bit
			return (__lin_flag_rx_error__ = RPIGPIO_RX_ERR_TIMING);
		}
		if (__lin_rx_level_change__(bit)) {
			// voltage level has changed, bit has been received
			bit = bit ? 0 : 1;
			if (bit_cnt >= 0)
				byte |= (bit << bit_cnt);
			bit_cnt ++;
			__lin_last_rx_tick__ = tick;
			continue;
		}
		// same voltage level, check timing
		if (t_elapsed > 52) {
			if (bit_cnt >= 0)
				byte |= (bit << bit_cnt);
			bit_cnt ++;
			__lin_last_rx_tick__ += 52;
			continue;
		}
	}
	// wait for stop bit
	while (!__lin_rx_level_change__(0)) {
		if (gpioTick() - __lin_last_rx_tick__ > __lin_tolerated_bit_time__)
			// did not receive stop-bit for more than 1 bit time
			return (__lin_flag_rx_error__ = RPIGPIO_RX_ERR_STOP_BIT);
	}
	*rxbyte = (unsigned char)(byte & 0xFF);
	return (0);
}

// Poll for a response frame
// 	<id> is the contains only the first 6 bits: the P0 and P1 protection will be calculated by this function
//	<len> is the number of data bytes expected
//	<data> is the array of bytes to received the data
// NOTE: <data> must be at least <len>+1 size.  The +1 is for receiving the checksum
// Function returns 0 on success, negative value on error
int rpigpio_lin_poll_frame (unsigned char id, int len, unsigned char * data)
{
	unsigned char pid = __lin_pid_lut__[id&0x3F];
	int byte;
	uint32_t chksum;
	
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	
	// start sending
	__lin_send_break_field__();
	__lin_send_byte__(0x55);		// SYNC field
	__lin_send_byte__(pid);			// PID field
	if (__lin_time_exceeded__ == 0) {
		for (byte=0; byte<=len; byte++) {
			if (__lin_rx_byte__(data+byte) < 0)
				break;
			chksum += data[byte];
		}
		chksum = (chksum + (chksum >> 8)) & 0xFF;
		if ((__lin_flag_rx_error__ == 0) && (chksum != 0xFF))
			__lin_flag_rx_error__ = RPIGPIO_RX_ERR_CHKSUM;
	} else 
		__lin_flag_rx_error__ = RPIGPIO_RX_ERR_HEADER;
	// return the LIN bus back to idle state (recessive = 1)
	__LIN_TX_BIT_1__;
	rpigpio_enable_interrupt();	
	return (__lin_flag_rx_error__);
}


// Default Timings
#define RPIGPIO_T_RANGING_USEC	17000	// 17msec
#define RPIGPIO_T_SAMPLING_USEC	33000	// 33msec
#define RPIGPIO_T_NEXT_USEC		50000	// 50msec
#define RPIGPIO_T_CYCLE_USEC	500000	// 500msec
#define RPIGPIO_T_SAMPLE_USEC	48		// 48usec


#define __CONFIGURE_GPIO(PIN,INOUT,PUPDN) \
	gpioSetMode(PIN,INOUT);	\
	gpioSetPullUpDown(PIN,PUPDN)

static RPI_CB_p __rpicb = NULL;
static char * __rpistr = NULL;

/* RPIGPIO control block nitialization
 * 	returns pointer to a new control block on successful initialization, 
 * 	NULL on error.
 */
RPI_CB_p rpigpio_init (void)
{
	if (!__rpicb) {
		int i;

		if (gpioInitialise() < 0)
			return (NULL);
		// set data bus to input
		__CONFIGURE_GPIO(RPIGPIO_D0, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D1, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D2, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D3, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D4, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D5, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D6, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D7, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
	
		// configure ADC pins
		__CONFIGURE_GPIO(RPIGPIO_ADC_BUSY, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_ADC_CS, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
#ifdef __VER_4_AIS_USE_ADC976__		
		__CONFIGURE_GPIO(RPIGPIO_ADC_RC, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_ADC_BYTE, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
#else 	// using ADC7819
		__CONFIGURE_GPIO(RPIGPIO_ADC_RD, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_ADC_CONVST, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
#endif	// ADC version

		// Configure LIN bus
		__CONFIGURE_GPIO(RPIGPIO_LIN_RX, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_LIN_TX, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_LIN_RW, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		
		__rpicb = (RPI_CB_p)calloc(1, sizeof(RPI_CB_t));
		__rpicb->usec_cycle = RPIGPIO_T_CYCLE_USEC;
		__rpicb->usec_next = RPIGPIO_T_NEXT_USEC;
		__rpicb->usec_sampling = RPIGPIO_T_SAMPLING_USEC;
		__rpicb->usec_ranging = RPIGPIO_T_RANGING_USEC;
		__rpicb->usec_sample = RPIGPIO_T_SAMPLE_USEC;
		
		for (i=0; i<NUM_SENSORS; i++)
			rpigpio_sensor_init(__rpicb->sensors+i, i);
			
		__rpistr = (char*)calloc(MAX_SAMPLES*32+256, sizeof(char));
	}
	return (__rpicb);
}

/* Cleanup the RPiGPIO
 */
void rpigpio_fini (void)
{
	int i;
	if (!__rpicb)
		return;
	for (i=0; i<NUM_SENSORS; i++)
		rpigpio_sensor_fini(__rpicb->sensors+i);
	if (__rpicb->ctrl_flags & RPICB_ENABLE_CAM)
		rpicam_fini();
	free (__rpicb);
	__rpicb = NULL;
	if (__rpistr)
		free (__rpistr);
}

/* Print information on RPi in JSON format
 * Calling function should not modify or deallocate the returned string
 * and should treat the returned string as only valid until the next call
 * to this function or to rpigpio_history_print().
 */
const char *  rpigpio_print (void)
{
	char * p = __rpistr;
	
	if (!__rpicb)
		return (NULL);
	p += sprintf(__rpistr, "{ \"sensors\": [ %d, %d, %d, %d ],", 
		__rpicb->sensors[0].enabled, __rpicb->sensors[1].enabled, 
		__rpicb->sensors[2].enabled, __rpicb->sensors[3].enabled);
	p += sprintf(p, "  \"T-ranging\": %d, \"T-sampling\": %d, \"T-next\": %d, \"T-cycle\": %d , \"T-sample\": %d }\n",
		__rpicb->usec_ranging, __rpicb->usec_sampling,
		__rpicb->usec_next, __rpicb->usec_cycle, __rpicb->usec_sample);
	return (__rpistr);
}

/* --------------------------------------------------------------------
 *  Common GPIO utilities
 * --------------------------------------------------------------------
 */

#ifdef __VER_2_CIRCUIT__

// set the address bits A0,A1
#define __address__(_SEN) \
	gpioSetBank1(RPIGPIO_ALE_bit); \
	gpioClearBank1(RPIGPIO_A2A3_bit); \
	gpioSetBank1(_SEN->A0A1_bits_set | RPIGPIO_ALE_bit); \
	gpioClearBank1(_SEN->A0A1_bits_clr)

// start ranging, assuming A0,A1 is already set 
// Ranging is A3=0, A2=1, ALE=0
#define __range__()	\
	gpioSetBank1(RPIGPIO_A2ALE_bit); \
	gpioClearBank1(RPIGPIO_A3ALE_bit)
 
// start conversion, assuming A0,A1 is already set 
// ADC conversion is A3=1, A2=0, ALE=0
#define __convert__()	\
	gpioSetBank1(RPIGPIO_A3ALE_bit); \
	gpioClearBank1(RPIGPIO_A2ALE_bit)
 
// latch the converted data to the data bus
// ADC latch is A3=1, A2=1, ALE=0
#define __latch__()	\
	gpioSetBank1(RPIGPIO_A2A3ALE_bit); \
	gpioClearBank1(RPIGPIO_ALE_bit)

#endif // __VER_2_CIRCUIT__

/* start ranging for sensor <id> */
void rpigpio_start_range (int id)
{
	if ((id >= 0) && (id < NUM_SENSORS)) {
		gpioSetBank1(__rpicb->sensors[id].A0A1_bits_set | RPIGPIO_ALE_bit);
		gpioClearBank1(__rpicb->sensors[id].A0A1_bits_clr);
		__range__();
	}
}

/* start conversion for sensor <id> */
void rpigpio_start_convert (int id)
{
	if ((id >= 0) && (id < NUM_SENSORS)) {
		gpioSetBank1(__rpicb->sensors[id].A0A1_bits_set | RPIGPIO_ALE_bit);
		gpioClearBank1(__rpicb->sensors[id].A0A1_bits_clr);
		__convert__();
	}
}

/* start latching for sensor <id> */
void rpigpio_start_latch (int id)
{
	if ((id >= 0) && (id < NUM_SENSORS)) {
		gpioSetBank1(__rpicb->sensors[id].A0A1_bits_set & RPIGPIO_ALE_bit);
		gpioClearBank1(__rpicb->sensors[id].A0A1_bits_clr);
		__latch__();
	}
}

#define __read_byte__(R) {	\
	uint32_t reg = gpioReadBank1();	\
	R |= (reg & RPIGPIO_D0_bit) >> RPIGPIO_D0_shift; \
	R |= (reg & RPIGPIO_D1D2_bit) >> RPIGPIO_D1D2_shift;	\
	R |= (reg & RPIGPIO_D3_bit) >> RPIGPIO_D3_shift;	\
	R |= (reg & RPIGPIO_D4567_bit) >> RPIGPIO_D4567_shift; \
}

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

/* Read data
 *  It is assumed that the address pins A0 & A1 are already set prior to this call
 *	For version 2, 8-bit data will be returned
 * 	For version 3, 16-bit data will be returned
 */
SAMPLE rpigpio_read_data (void)
{
	uint32_t res=0;

#ifdef __VER_2_CIRCUIT__
	while (gpioReadBank1() & RPIGPIO_BUSY_bit)
		rpi_usleep(1);
	__latch__();
	rpi_usleep(1);			// ensure enough time for data to appear on databus
	__read_byte__(res);
	return (res & 0xFF);
#else
	// set BYTE=0 : Low Byte
	__read_byte__(res);
	// set BYTE=0 : High Byte
	__read_byte__(res);	
	return (res & 0xFFFF);
#endif // __VER_2_CIRCUIT__
}

/* compare timing:
 *	Return 0 if <period> is exceeded since <last>, remaining usec otherwise
 *  <last> and <period> are in usec
 */
static inline int __rpigpio_time_remaining (uint32_t now, uint32_t last, uint32_t period) 
{
	now -= last;
	return ((now < period) ? period - now : 0);
}

/* compare timing:
 *	Return 0 if <period> is exceeded since <last>, remaining usec otherwise
 *  <last> and <period> are in usec
 */
int rpigpio_time_remaining (uint32_t last, uint32_t period) 
{
	return __rpigpio_time_remaining(gpioTick(), last, period);
}

/*
 * Debug GPIO pins
 */
void rpigpio_pins_show (void)
{
	uint32_t reg = gpioReadBank1();
	uint32_t A0, A1, A2, A3, ALE, D0, D1, D2, D3, D4, D5, D6, D7, BUSY;
	A0 = (reg & RPIGPIO_A0_bit) ? 1 : 0;
	A1 = (reg & RPIGPIO_A1_bit) ? 1 : 0;
	A2 = (reg & RPIGPIO_A2_bit) ? 1 : 0;
	A3 = (reg & RPIGPIO_A3_bit) ? 1 : 0;
	ALE = (reg & RPIGPIO_ALE_bit) ? 1 : 0;
	D0 = (reg & RPIGPIO_D0_bit) ? 1 : 0;
	D1 = (reg & RPIGPIO_D1_bit) ? 1 : 0;
	D2 = (reg & RPIGPIO_D2_bit) ? 1 : 0;
	D3 = (reg & RPIGPIO_D3_bit) ? 1 : 0;
	D4 = (reg & RPIGPIO_D4_bit) ? 1 : 0;
	D5 = (reg & RPIGPIO_D5_bit) ? 1 : 0;
	D6 = (reg & RPIGPIO_D6_bit) ? 1 : 0;
	D7 = (reg & RPIGPIO_D7_bit) ? 1 : 0;
	BUSY = (reg & RPIGPIO_BUSY_bit) ? 1 : 0;
	printf("A0[%d] A1[%d] A2[%d] A3[%d] ALE[%d] D[%d%d%d%d %d%d%d%d] BUSY[%d]\n",
		A0, A1, A2, A3, ALE, D7, D6, D5, D4, D3, D2, D1, D0, BUSY);
}

/* --------------------------------------------------------------------
 *  Sensor Control Block
 * --------------------------------------------------------------------
 */

/* Initialize a sensor control block <sen>
 * If <sen> is NULL, a new control block will be allocated
 * <id> is the id of the sensor, to determine the address signals
 * Return <sen> on sucess
 */
SENSOR_p rpigpio_sensor_init (SENSOR_p sen, int id)
{
	int A0 = (id &1), A1 = (id >> 1) & 1;
	
	if ((id < 0) || (id >= NUM_SENSORS))
		return NULL;
		
	if (!sen)
		sen = (SENSOR_p)calloc(1, sizeof(SENSOR_t));
		
	sen->sensorid = id;
	sen->enabled = 0;
#ifdef __arm__
	sen->usec_last_range = gpioTick();
#endif
	sen->A0A1_bits_set = (id & 1) ? RPIGPIO_A0_bit : 0;
	sen->A0A1_bits_set |= (id & 2) ? RPIGPIO_A1_bit : 0;
	sen->A0A1_bits_clr = (id & 1) ? 0 : RPIGPIO_A0_bit;
	sen->A0A1_bits_clr |= (id & 2) ? 0 : RPIGPIO_A1_bit;
	sen->curr = rpigpio_history_init(NULL, MAX_SAMPLES);
	sen->prev = NULL;

	return (sen);
}

/* Release resources held by sensor <sen>
 * Note that this only dellocates sample history buffer.
 * It does not deallocate the sensor control block itself
 */
void rpigpio_sensor_fini (SENSOR_p sen)
{
	if (sen) {
		SHISTORY_p p = sen->prev;
		if (sen->curr)
			rpigpio_history_free(sen->curr);
		while (p) {
			p = sen->prev->next;
			rpigpio_history_free (sen->prev);
			sen->prev = p;
		}
		sen->curr = sen->prev = NULL;
	}
}

#define __CONVERT__(_SEN)	\
	__address__(_SEN);	\
	__convert__()
	
#define __READDATA__(_SEN,_SAMP,_TS)	\
	__address__(_SEN); \
	_SAMP->samples[_SAMP->num_samples].timestamp = _TS; \
	_SAMP->samples[_SAMP->num_samples++].value = rpigpio_read_data()

#define __DO_IDLE__(_SEN,_TIME) { \
	tr = rpigpio_time_remaining(_SEN->usec_last_range, __rpicb->_TIME); \
	if (idle && (tr > __rpicb->idle_cb_usec)) \
		idle(cb_data, _SEN->usec_last_range, __rpicb->_TIME, _SEN); \
	tr = rpi_usleep(rpigpio_time_remaining(_SEN->usec_last_range, __rpicb->_TIME)); \
}


/* Start a cycle of ranging and sampling by sensor <sen>
 * Returns a pointer to the collected sample history
 * NULL on error (not enabled, or <sen> is NULL)
 */
SHISTORY_p rpigpio_sensor_range_and_sample_cb (SENSOR_p sen, IDLECALLBACK_f idle, void * cb_data)
{
	SHISTORY_p p;
	SAMPLE_p samp;
	uint32_t st, tr;
	
	if (sen && sen->enabled) {
		// initialize address bus
		__address__(sen);
		// ensure T_cycle is over before ranging
		__DO_IDLE__(sen, usec_cycle)
		sen->usec_last_range = tr;
		__range__();
		p = rpigpio_history_init (sen->curr, MAX_SAMPLES);
		p->sensorid = sen->sensorid;
		__CONVERT__(sen);
		//__convert__(); // activate ADC
		// capture image if required
		if (__rpicb->ctrl_flags & RPICB_ENABLE_CAM)
			rpicam_capture();
		// wait for T_range to be over
		__DO_IDLE__(sen, usec_ranging)
		p->usec_start_time = st = tr;
		// collect data
		p->sec_start_time = (uint32_t) time(NULL);
		__CONVERT__(sen);
		//__convert__();
		rpigpio_disable_interrupt();
		while (__rpigpio_time_remaining(st, p->usec_start_time, __rpicb->usec_sampling) > 0) {
			__READDATA__(sen,p,st);
			//p->samples[p->num_samples].timestamp = st;
			//p->samples[p->num_samples++].value = rpigpio_read_data();	// read data from last conversion
			st = rpi_usleep(rpigpio_time_remaining(st, __rpicb->usec_sample));
			__CONVERT__(sen);
			//__convert__();												// start conversion for next iteration
			if (p->num_samples >= MAX_SAMPLES)
				break;
		}
		gpioSetBank1(RPIGPIO_ALE_bit);	// disable ALE
		rpigpio_enable_interrupt();
		// debug("---- %s: returning\n", __func__);
		return (sen->curr = p);
	}
	return (NULL);
}

/* Start a cycle of ranging and sampling by sensor <sen>
 * Returns a pointer to the collected sample history
 * NULL on error (not enabled, or <sen> is NULL)
 */
inline SHISTORY_p rpigpio_sensor_range_and_sample (SENSOR_p sen)
{
	return rpigpio_sensor_range_and_sample_cb(sen, NULL, NULL);
/*
	SHISTORY_p p;
	SAMPLE_p samp;
	uint32_t st;
	
	if (sen && sen->enabled) {
		// initialize address bus
		__address__(sen);
		// ensure T_cycle is over before ranging
		sen->usec_last_range = rpi_usleep(rpigpio_time_remaining(sen->usec_last_range, __rpicb->usec_cycle));
		__range__();
		p = rpigpio_history_init (sen->curr, MAX_SAMPLES);
		p->sensorid = sen->sensorid;
		__CONVERT__(sen);
		//__convert__(); // activate ADC
		// capture image if required
		if (__rpicb->ctrl_flags & RPICB_ENABLE_CAM)
			rpicam_capture();
		// wait for T_range to be over
		p->usec_start_time = st = rpi_usleep(rpigpio_time_remaining(sen->usec_last_range, __rpicb->usec_ranging));
		// collect data
		p->sec_start_time = (uint32_t) time(NULL);
		__CONVERT__(sen);
		//__convert__();
		rpigpio_disable_interrupt();
		while (__rpigpio_time_remaining(st, p->usec_start_time, __rpicb->usec_sampling) > 0) {
			__READDATA__(sen,p,st);
			//p->samples[p->num_samples].timestamp = st;
			//p->samples[p->num_samples++].value = rpigpio_read_data();	// read data from last conversion
			st = rpi_usleep(rpigpio_time_remaining(st, __rpicb->usec_sample));
			__CONVERT__(sen);
			//__convert__();												// start conversion for next iteration
			if (p->num_samples >= MAX_SAMPLES)
				break;
		}
		gpioSetBank1(RPIGPIO_ALE_bit);	// disable ALE
		rpigpio_enable_interrupt();
		// debug("---- %s: returning\n", __func__);
		return (sen->curr = p);
	}
	return (NULL);
*/
}

/* Start a cycle of ranging for sensir <sen> 
 * and collect samples from ALL sensors (regardless if enabled)
 * Returns -1 on error (not enabled, or <sen> is NULL)
 * The readings are stored in each sensor's curr pointer
 */
int rpigpio_sensor_range_and_sample_all (SENSOR_p sen)
{
	SENSOR_p senp;
	SHISTORY_p p;
	SAMPLE_p samp;
	uint32_t ust, sst;
	int i;
	
	if (sen && sen->enabled) {
		// initialize address bus
		__address__(sen);
		// ensure T_cycle is over before ranging
		sen->usec_last_range = rpi_usleep(rpigpio_time_remaining(sen->usec_last_range, __rpicb->usec_cycle));
		__range__();
		for (senp=__rpicb->sensors, i=0; i<NUM_SENSORS; i++, senp++) {
			senp->curr = rpigpio_history_init (senp->curr, MAX_SAMPLES);
			__CONVERT__(senp);
			//__address__(senp);
			//__convert__(); // activate ADC
		}
		// capture image if required
		if (__rpicb->ctrl_flags & RPICB_ENABLE_CAM)
			rpicam_capture();
		// wait for T_range to be over
		ust = rpi_usleep(rpigpio_time_remaining(sen->usec_last_range, __rpicb->usec_ranging));
		sst = (uint32_t) time(NULL);
		for (senp=__rpicb->sensors, i=0; i<NUM_SENSORS; i++, senp++) {			
			senp->curr->sensorid = sen->sensorid;
			senp->curr->usec_start_time = ust;
			senp->curr->sec_start_time = sst;
			__CONVERT__(senp);
			//__address__(senp);
			//__convert__();  // convert
		}
		rpigpio_disable_interrupt();
		while (__rpigpio_time_remaining(ust, sen->curr->usec_start_time, __rpicb->usec_sampling) > 0) {
			for (senp=__rpicb->sensors, i=0; i<NUM_SENSORS; i++, senp++) {
				// read data from last conversion
				//__address__(senp);
				//senp->curr->samples[senp->curr->num_samples].timestamp = ust;
				//senp->curr->samples[senp->curr->num_samples++].value = rpigpio_read_data();
				__READDATA__(senp,senp->curr,ust);
			}
			ust = rpi_usleep(rpigpio_time_remaining(ust, __rpicb->usec_sample));
			for (senp=__rpicb->sensors, i=0; i<NUM_SENSORS; i++, senp++) {
				// start conversion for next iteration
				//__address__(senp);
				//__convert__();
				__CONVERT__(senp);
			}
			if (sen->curr->num_samples >= MAX_SAMPLES)
				break;
		}
		gpioSetBank1(RPIGPIO_ALE_bit);	// disable ALE
		rpigpio_enable_interrupt();
		// debug("---- %s: returning\n", __func__);
		return (0);
	}
	return (-1);
}


/* Move current history to previous.
 * If <max> is greater than 1, maximum number of samples history in previous
 * will be <max>.  Older histories will be removed.
 */
void rpigpio_sensor_update_history (SENSOR_p sen, int max)
{
	if (!sen || !sen->curr)
		return;
	if (!sen->prev) {
		sen->prev = sen->curr;
	} else {
		SHISTORY_p p;
		int cnt=0;
		// get to the last history
		for (cnt=0, p=sen->prev; p->next; p=p->next, cnt++);
		// append to the last history
		p->next = sen->curr;
		// check if we need to purge old histories
		if ((max > 1) && (cnt > max)) {
			for (; cnt>max; cnt--) {
				p = sen->prev;
				sen->prev = p->next;
				rpigpio_history_free(p);
			}
		}
	}
	sen->curr = NULL;
}

/* Move current history to previous, and set the history ID
 * If <max> is greater than 1, maximum number of samples history in previous
 * will be <max>.  Older histories will be removed.
 */
void rpigpio_sensor_update_history_with_id (SENSOR_p sen, int max, int histid)
{
	if (!sen || !sen->curr)
		return;
	sen->curr->histid = histid;
	if (!sen->prev) {
		sen->prev = sen->curr;
	} else {
		SHISTORY_p p;
		int cnt=0;
		// get to the last history
		for (cnt=0, p=sen->prev; p->next; p=p->next, cnt++);
		// append to the last history
		p->next = sen->curr;
		// check if we need to purge old histories
		if ((max > 1) && (cnt > max)) {
			for (; cnt>max; cnt--) {
				p = sen->prev;
				sen->prev = p->next;
				rpigpio_history_free(p);
			}
		}
	}
	sen->curr = NULL;
}
 
/* --------------------------------------------------------------------
 *  Sample History
 * --------------------------------------------------------------------
 */

/* Initialize a sample history <hist>
 * If <hist> is NULL, a new sample history will be allocated
 * If <size> is <=0, no new sample buffer will be allocated
 * Return <hist> on sucess
 */
SHISTORY_p rpigpio_history_init (SHISTORY_p hist, int size)
{
	if (!hist)
		hist = (SHISTORY_p)calloc(1, sizeof(SHISTORY_t));
		
	hist->sec_start_time = 0;
	hist->usec_start_time = 0;
	hist->num_samples = 0;
	hist->next = 0;
	if (size >= 0)
		hist->samples = (SAMPLE_p)realloc(hist->samples, size * sizeof(SAMPLE_t));
	
	return (hist);
	
}

/* Deallocates a sample history <hist>
 * The next pointer is ignored.
 */
void rpigpio_history_free (SHISTORY_p hist)
{
	if (hist->samples)
		free (hist->samples);
	free (hist);
}

/* prints the samples to a string in JSON format
 * Calling function should not modify or deallocate the returned string
 * and should treat the returned string as only valid until the next call
 * to this function or to rpigpio_print().
 */
const char * rpigpio_history_print (SHISTORY_p hist, int senid)
{
	char * p;
	unsigned pt;
	int i, tdiff;
	
	if (!hist || (hist->num_samples <= 0))
		return (NULL);
		
	p += sprintf (p=__rpistr, "{  \"id\": %d, \"start\": [%u, %u], \"len\": %d, \"recv-sensor\": %d, \"firing-sensor\": %d,\n  \"samples\": [", 
			hist->histid, hist->sec_start_time, hist->usec_start_time % 1000000, hist->num_samples, senid, hist->sensorid);
	
	for (i=0, pt=hist->samples->timestamp; i<hist->num_samples; i++) {
		tdiff = (pt <= hist->samples[i].timestamp) ? (hist->samples[i].timestamp - pt) :
			((hist->samples[i].timestamp & 0xfffffff) | 0x10000000) - (pt & 0xfffffff);
		if (!(i%10))
			p += sprintf(p, "\n    ");
		p += sprintf(p, "[%5d, %5d], ", tdiff, hist->samples[i].value);
		pt = hist->samples[i].timestamp;
	}
	sprintf(p, "\n] }\n");
	return (__rpistr);
}
