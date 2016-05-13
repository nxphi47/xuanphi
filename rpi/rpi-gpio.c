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

#ifdef __VER_2_CIRCUIT__
/* using version 2.0 circuit */

// address pins
#define RPIGPIO_A0		5
#define RPIGPIO_A1		6
#define RPIGPIO_A2		13
#define RPIGPIO_A3		26
#define RPIGPIO_ALE		19

#define RPIGPIO_A0_bit		0x00000020	// 1<<5
#define RPIGPIO_A1_bit		0x00000040	// 1<<6
#define RPIGPIO_A2_bit		0x00002000	// 1<<13
#define RPIGPIO_A3_bit		0x04000000	// 1<<26
#define RPIGPIO_ALE_bit		0x00080000	// 1<<19

#define RPIGPIO_A2A3_bit	0x04002000
#define RPIGPIO_A2ALE_bit	0x00082000
#define RPIGPIO_A3ALE_bit	0x04080000
#define RPIGPIO_A2A3ALE_bit	0x04082000

// data pins
#define RPIGPIO_D0		4
#define RPIGPIO_D1		17
#define RPIGPIO_D2		18
#define RPIGPIO_D3		27
#define RPIGPIO_D4		22
#define RPIGPIO_D5		23
#define RPIGPIO_D6		24
#define RPIGPIO_D7		25

#define RPIGPIO_D0_bit		0x00000010	// 1<<4
#define RPIGPIO_D1_bit		0x00020000	// 1<<17
#define RPIGPIO_D2_bit		0x00040000	// 1<<18
#define RPIGPIO_D3_bit		0x08000000	// 1<<27
#define RPIGPIO_D4_bit		0x00400000	// 1<<22
#define RPIGPIO_D5_bit		0x00800000	// 1<<23
#define RPIGPIO_D6_bit		0x01000000	// 1<<24
#define RPIGPIO_D7_bit		0x02000000	// 1<<25

#define RPIGPIO_D0_shift	4
#define RPIGPIO_D1_shift	16
#define RPIGPIO_D2_shift	16
#define RPIGPIO_D3_shift	24
#define RPIGPIO_D4_shift	18
#define RPIGPIO_D5_shift	18
#define RPIGPIO_D6_shift	18
#define RPIGPIO_D7_shift	18

#define RPIGPIO_D1D2_bit	0x00060000
#define RPIGPIO_D1D2_shift	16
#define RPIGPIO_D4567_bit	0x03C00000
#define RPIGPIO_D4567_shift	18

#define RPIGPIO_BUSY		12

#define RPIGPIO_BUSY_bit	0x00001000 // 1<<12

#endif // __VER_2_CIRCUIT__

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
#ifdef __arm__
		if (gpioInitialise() < 0)
			return (NULL);
		__CONFIGURE_GPIO(RPIGPIO_A0, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_A1, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_A2, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_A3, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_ALE, RPIGPIO_OUTPUT, RPIGPIO_PUD_OFF);
		__CONFIGURE_GPIO(RPIGPIO_D0, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D1, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D2, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D3, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D4, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D5, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D6, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_D7, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		__CONFIGURE_GPIO(RPIGPIO_BUSY, RPIGPIO_INPUT, RPIGPIO_PUD_DOWN);
		gpioWrite(RPIGPIO_ALE, 1);	// disable ALE
#endif
		
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
	p += sprintf(__rpistr, "{ 'sensors': [ %d, %d, %d, %d ],", 
		__rpicb->sensors[0].enabled, __rpicb->sensors[1].enabled, 
		__rpicb->sensors[2].enabled, __rpicb->sensors[3].enabled);
	p += sprintf(p, "  'T-ranging': %d, 'T-sampling': %d, 'T-next': %d, 'T-cycle': %d , 'T-sample': %d }\n",
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
inline uint32_t rpi_usleep(int n) 
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
inline int __rpigpio_time_remaining (uint32_t now, uint32_t last, uint32_t period) 
{
	now -= last;
	return ((now < period) ? period - now : 0);
}

/* compare timing:
 *	Return 0 if <period> is exceeded since <last>, remaining usec otherwise
 *  <last> and <period> are in usec
 */
inline int rpigpio_time_remaining (uint32_t last, uint32_t period) 
{
	__rpigpio_time_remaining(gpioTick(), last, period);
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

/* Start a cycle of ranging and sampling by sensor <sen>
 * Returns a pointer to the collected sample history
 * NULL on error (not enabled, or <sen> is NULL)
 */
SHISTORY_p rpigpio_sensor_range_and_sample (SENSOR_p sen)
{
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
		
	p += sprintf (p=__rpistr, "{  'id': %d, 'start': [%u, %u], 'len': %d, 'recv-sensor': %d, 'firing-sensor': %d,\n  'samples': [", 
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
