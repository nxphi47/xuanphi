/*
	Hardware Routines for AIS Sensor Firing and Sampling
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
#include <string.h>

// ------------- Low Level BCM8235 Interface -----------------
#include "bcm2835.h"
#include "hw.h"

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

// ------------------ LED ----------------------

typedef struct __led_status__ {
	uint32_t gpio;				// GPIO number
	uint32_t state;				// status
	uint32_t ts;				// timestamp
} LED_t, * LED_p;

#define LED_POWER_GPIO		5		// GPIO# for Power LED
#define LED_BURST_GPIO		6		// GPIO# for Bursting LED
#define LED_LINCOMMS_GPIO	13		// GPIO# for LIN Comms

#define LED_STATE_CURR			0x0001		// current state
#define LED_SLOW_BLINK_RATE		400000		// 400msec on/off = 0.8s cycle
#define LED_FAST_BLINK_RATE		150000		// 150msec on/off = 0.3s cycle

// static structure for LEDs
static LED_t __leds[] = {
	{ gpio: LED_POWER_GPIO,    state: LED_STATE_OFF,  ts: 0 },		// LED_POWER
	{ gpio: LED_BURST_GPIO,    state: LED_STATE_OFF,  ts: 0 },		// LED_BURST
	{ gpio: LED_LINCOMMS_GPIO, state: LED_STATE_OFF, ts: 0 },		// LED_LINCOMMS	
};
#define NUM_LEDS	3

// Exposed to outside module
void * LED_POWER = (void*)&(__leds[0]);
void * LED_BURST = (void*)&(__leds[1]);
void * LED_LINCOMMS = (void*)&(__leds[2]);

// initialization
void LED_init (void) 
{
	int i;
	
	// configure the GPIO
	for (i=0; i<NUM_LEDS; i++) {
		bcm2835_gpio_fsel(__leds[i].gpio, BCM2835_GPIO_FSEL_OUTP);
		__gpio_clr__(1 << __leds[i].gpio);
		__leds[i].state = LED_STATE_OFF;
		__leds[i].ts = __sys_tick__();
	}
}

// set the state of the specified LED using one of the define states:
//		ON, OFF, SLOW_BLINK, FAST_BLINK
void LED_set_state ( void * led, uint32_t state ) 
{
	LED_p p = (LED_p)led;
	if (p) {
		p->state &= 0x00ff;
		p->state |= state & 0xff00;
	}
}

// periodic callback
void LED_timeout ( void ) 
{
	int i;
	
	for (i=0; i<NUM_LEDS; i++)
		if (__leds[i].state & LED_STATE_OFF) {
			__gpio_clr__(1<<__leds[i].gpio);
			__leds[i].state = LED_STATE_OFF;
			__leds[i].ts = __sys_tick__();
		} 
		else if (__leds[i].state & LED_STATE_ON) {
			__gpio_set__(1<<__leds[i].gpio);
			__leds[i].state = LED_STATE_ON | LED_STATE_CURR;
			__leds[i].ts = __sys_tick__();
		}
		else {
			if (((__leds[i].state & LED_STATE_SLOW_BLINK) && (__sys_tick__() - __leds[i].ts > LED_SLOW_BLINK_RATE)) ||
				((__leds[i].state & LED_STATE_FAST_BLINK) && (__sys_tick__() - __leds[i].ts > LED_FAST_BLINK_RATE))) {
					// toggle
					if (__leds[i].state & LED_STATE_CURR) {
						__gpio_clr__(1<<__leds[i].gpio);
						__leds[i].state &= ~LED_STATE_CURR;
					} else { 
						__gpio_set__(1<<__leds[i].gpio);
						__leds[i].state |= LED_STATE_CURR;
					}
					__leds[i].ts = __sys_tick__();
			}
		}
}

// clean_up
void LED_close (void) 
{
	int i;
	// configure the GPIO to INPUT and PUD_DOWN
	for (i=0; i<NUM_LEDS; i++) {
		__gpio_clr__(1 << __leds[i].gpio);
		bcm2835_gpio_fsel(__leds[i].gpio, BCM2835_GPIO_FSEL_INPT);
		bcm2835_gpio_set_pud(__leds[i].gpio, BCM2835_GPIO_PUD_DOWN);
	}
}


// ------------------ LIN -----------------------

#define __serial_dev "/dev/ttyAMA0"

#define LIN_PIN_BREAK	18			// set to 0 to send break signal (header pin 12)
#define LIN_PIN_TX_EN	4			// Enable TX (Header pin 7)
#define LIN_PIN_RX_EN	17			// Eanble RX (Header pin 11)
#define LIN_PIN_TX		14			// TX0 on GPIO (Header pin 8) 
#define LIN_PIN_RX		15			// RX0 on GPIO (Header pin 10)


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
	
#ifdef __DEBUG__
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
#endif

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
	unsigned char byte = (unsigned char)(B);\
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

#ifdef __DEBUG__
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
#endif

// ------------------- ADC -----------------------

// initialize the ADC
void ADC_init (void) 
{
	// MC3202 has a requirement of minimum 10kHz to maintain the stored charge for 1.2ms
	// we are sampling at ~48usec, and each sampling takes 3 bytes (24bits) of SPI communications
	// Hence we should set the speed at minimum 24/48e-6 = 500kHz
	// Let's play save and use 1.953MHz
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

// ------------- Convenient Routines -------------------------

// initializes the LED, LIN and ADC interface
int HW_init (void)
{
	if (bcm2835_init() != 1) {
		printf ("bcm2835_init() failed.\n");
		return (-1);
	}
	if (LIN_init() < 0)
		return (-2);
	ADC_init();
	LED_init();
	HW_check();
	return (0);
}

// closes the LED, LIN and ADC interface
void HW_close (void)
{
	LED_close();
	LIN_close();
	ADC_fini();
	bcm2835_close();
}

// Get the current mircoseconds tick
uint32_t HW_tick (void)
{
	return __sys_tick__();
}

// Sleep for specified microseconds
uint32_t HW_usleep (uint32_t usec)
{
	volatile uint32_t* paddr = bcm2835_st + BCM2835_ST_CLO/4;
	uint32_t st, t;
	
	st = t = *paddr;
	if (usec > 300)
		usleep(usec-150);
	for (; t - st < usec; t = *paddr);
	return (*paddr);
}

uint32_t hw_state = 0x00;
static uint32_t __last_hw_check = 0;

#define ADC_LOWEST_LEVEL	50
#define ADC_HIGH_LEVEL		4050

#ifdef __DEBUG__
static int __hw_check_first_check=1;
#define __hw_check_out(fmt...)         \
	if (__hw_check_first_check)    \
		fprintf(stderr, ##fmt)
#else
#define __hw_check_out(fmt...)
#endif

// Check the current status of hardware
uint32_t HW_check (void)
{
	uint32_t tmp;
	
	if ((__last_hw_check == 0) || (__sys_tick__() - __last_hw_check > 5000000)) {
		hw_state = 0x00;
		// test ADC first
		tmp = ADC_read();
		if (tmp < ADC_LOWEST_LEVEL)
			hw_state |= HW_ADC_LOW;
		if (tmp > ADC_HIGH_LEVEL)
			hw_state |= HW_ADC_HIGH;
		__hw_check_out("HW-Check: ADC_read()=%d\n", tmp);

		// test LIN
		bcm2835_gpio_fsel(LIN_PIN_RX, BCM2835_GPIO_FSEL_INPT); /* RX0 */
		bcm2835_gpio_set_pud(LIN_PIN_RX, BCM2835_GPIO_PUD_OFF);
		__gpio_set__(1<<LIN_PIN_RX_EN);
		// we should read a HIGH in RX
		usleep(1);
		tmp = __gpio_level__();
		if (tmp & (1<<LIN_PIN_RX) == 0)
			hw_state |= HW_LIN_RX_LOW;
		__hw_check_out("HW-Check: RX: LIN_READ()=%x\n", tmp & (1<<LIN_PIN_RX));
		// we should read a low in RX
		__gpio_clr__(LIN_PIN_BREAK);
		usleep(1);
		tmp = __gpio_level__();
		if (tmp & (1<<LIN_PIN_RX) != 0)
			hw_state |= HW_LIN_BREAK_HIGH;
		__hw_check_out("HW-Check: BRK: LIN_READ()=%x\n", tmp & (1<<LIN_PIN_RX));
	  
		// return back to normal setting
		bcm2835_gpio_fsel(LIN_PIN_RX, BCM2835_GPIO_FSEL_ALT0); /* RX0 */
		__LIN_SET_RX();

		// set the HW state and LED accordingly
		if (hw_state & (HW_ADC_LOW | HW_LIN_BREAK_HIGH | HW_LIN_RX_LOW))
			hw_state |= HW_NO_12V_SUPPLY;
		if (hw_state & HW_ADC_HIGH)
			hw_state |= HW_NO_SENSOR;
			
		if (hw_state & HW_NO_12V_SUPPLY)
			LED_set_state( LED_POWER, LED_STATE_FAST_BLINK);
		else if (hw_state & HW_NO_SENSOR)
			LED_set_state( LED_POWER, LED_STATE_SLOW_BLINK);
		else // No problem
			LED_set_state( LED_POWER, LED_STATE_ON);
			
		__last_hw_check = __sys_tick__();
#ifdef __DEBUG__
		__hw_check_first_check=0;
#endif
	}
}

#ifdef __DEBUG__

typedef struct __hw_debug_pins__ {
	uint32_t pin;
	char name[8];
} HWDBG_PIN_t, * HWDBG_PIN_p;

static HWDBG_PIN_t hw_debug_pins[] = {
	{ pin: LIN_PIN_BREAK, name: "BREAK" }, 
	{ pin: LIN_PIN_RX,    name: "RX" }, 
	{ pin: LIN_PIN_RX_EN, name: "RX-EN" }, 
	{ pin: LIN_PIN_TX,    name: "TX" }, 
	{ pin: LIN_PIN_TX_EN, name: "TX_EN" }, 
	{ pin: LED_POWER_GPIO, name: "LED0" }, 
	{ pin: LED_BURST_GPIO, name: "LED1" }, 
	{ pin: LED_LINCOMMS_GPIO, name: "LED2" }, 
	{ pin: 0xff, name: '\0' }
};
static char hw_debug_str [128]= "";

// initialize hw debugger
void HW_DBG_init (void)
{
	// disconnect serial and set TX=OUT, RX=IN
	bcm2835_gpio_fsel(LIN_PIN_TX, BCM2835_GPIO_FSEL_OUTP); /* TX0 */
	bcm2835_gpio_fsel(LIN_PIN_RX, BCM2835_GPIO_FSEL_INPT); /* RX0 */
}

// close the hw debugger
void HW_DBG_close (void)
{
	// connect back serial
	bcm2835_gpio_fsel(LIN_PIN_TX, BCM2835_GPIO_FSEL_ALT0); /* TX0 */
	bcm2835_gpio_fsel(LIN_PIN_RX, BCM2835_GPIO_FSEL_ALT0); /* RX0 */
}

// show the current pin status
const char * HW_DBG_show (void)
{
	int i, n;
	uint32_t gpio = __gpio_level__();
	
	n = sprintf(hw_debug_str, "GPIO=0x%04x ", gpio);
	for (i=0; hw_debug_pins[i].pin < 32; i++)
		n += sprintf(hw_debug_str+n, "%s=%d ", hw_debug_pins[i].name, gpio & (1<<hw_debug_pins[i].pin)?1:0);
	return (hw_debug_str);
}

// set the pins levels
void HW_DBG_set ( const char * str )
{
	uint32_t clr=0, set=0;
	const char * p;
	int i;
	
	for (p=str; *p; p++) {
		if ((*p == ' ') || (*p == '\t'))
			continue;
		for (i=0; hw_debug_pins[i].pin < 32; i++)
			if (strncasecmp(p, hw_debug_pins[i].name, strlen(hw_debug_pins[i].name)) == 0) {
				p += strlen(hw_debug_pins[i].name) + 1;
				if (*p == '0')
					clr |= (1<<hw_debug_pins[i].pin);
				else if (*p == '1')
					set |= (1<<hw_debug_pins[i].pin);
				for (; *p != ' '; p++);
				break;
			}
	}
	printf ("-- HW-DBG: STR: '%s' ==> SET=0x%04x, CLR=0x%04x\n", str, set, clr);
	__gpio_clr_n_set__(clr, set);
}

#endif // __DEBUG__
