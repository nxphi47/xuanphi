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
#define SPI0_LEN	0x18

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
static volatile uint32_t  *__spi0Reg = MAP_FAILED;

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
	__spi0Reg  = initMapMem(fd, SPI0_BASE,  SPI0_LEN);
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



/* -------------- SPI ------------------- */

/* Registers offsets */
#define SPI0_REG_CS		0
#define SPI0_REG_FIFO	0x04
#define SPI0_REG_CLK	0x08
#define SPI0_REG_DLEN	0x0c
#define SPI0_REG_LTOH	0x10
#define SPI0_REG_DMA	0x14

/* Control Status bits */
#define SPI0_CS_CS01		0x00000001	// bit 0
#define SPI0_CS_CS10		0x00000002	// bit 1
#define SPI0_CS_CLKPHA		0x00000004	// bit 2 
#define SPI0_CS_CLKPOL		0x00000008	// bit 3
#define SPI0_CS_TXCLR		0x00000010	// bit 4
#define SPI0_CS_RXCLR		0x00000020	// bit 5 
#define SPI0_CS_CSPOL		0x00000040	// bit 6
#define SPI0_CS_ACTIVE		0x00000080	// bit 7
#define SPI0_CS_TXDONE		0x00010000	// bit 16
#define SPI0_CS_RXDATA		0x00020000	// bit 17
#define SPI0_CS_TXRDY		0x00040000	// bit 18
#define SPI0_CS_RXRDY		0x00080000	// bit 19
#define SPI0_CS_RXFULL		0x00100000	// bit 20
#define SPI0_CS_CSPOL0		0x00200000	// bit 21
#define SPI0_CS_CSPOL1		0x00400000	// bit 22
#define SPI0_CS_CSPOL2		0x00800000	// bit 23

#define SPI0_CS_FIFOCLR		0x00000030

/* SPI Pins to GPIO# Mapping */
#define SPI0_PIN_MOSI	10
#define SPI0_PIN_MISO	9
#define SPI0_PIN_SCLK	11
#define SPI0_PIN_CS0	8
#define SPI0_PIN_CS1	7

/* Initialization flags for SPI configuration */
#define RPIGPIO_SPI_CS_0				0
#define RPIGPIO_SPI_CS_1				SPI0_CS_CS01
#define RPIGPIO_SPI_CS_2				SPI0_CS_CS10
#define RPIGPIO_SPI_CLK_PHASE_MIDDLE	0
#define RPIGPIO_SPI_CLK_PHASE_BEGIN		SPI0_CS_CLKPHA
#define RPIGPIO_SPI_CLK_POL_LOW			0
#define RPIGPIO_SPI_CLK_POL_HIGH		SPI0_CS_CLKPOL
#define RPIGPIO_SPI_CS_POL_LOW			0
#define RPIGPIO_SPI_CS_POL_HIGH			SPI0_CS_CSPOL

/* Speed of SPI Bus */
typedef enum {
	RPIGPIO_SPI_9kHz	= 2048,
	RPIGPIO_SPI_18kHz	= 1024,
	RPIGPIO_SPI_37kHz	= 512,
	RPIGPIO_SPI_75kHz	= 256,
	RPIGPIO_SPI_150kHz	= 128,
	RPIGPIO_SPI_300kHz	= 64,
	RPIGPIO_SPI_600kHz	= 32,
	RPIGPIO_SPI_1200kHz	= 16,
	RPIGPIO_SPI_2400kHz	= 8,
	RPIGPIO_SPI_4800kHz	= 4,
	RPIGPIO_SPI_9600kHz	= 2,
	RPIGPIO_SPI_5kHz	= 1
} RPIGPIO_SPI0_SPEED_t;


#define __CONFIGURE_GPIO(PIN,INOUT,PUPDN) \
	gpioSetMode(PIN,INOUT);	\
	gpioSetPullUpDown(PIN,PUPDN)

static uint32_t __spi0_cs_value = 0x00;
static int __spi0_initialized = 0;
#define __SET_SPI0_CS(BIT)		__spi0Reg[SPI0_REG_CS] = __spi0_cs_value | (BIT)
#define __CLR_SPI0_CS(BIT)		__spi0Reg[SPI0_REG_CS] = __spi0_cs_value & ~(BIT)
#define __GET_SPI0_CS(BIT)		(__spi0Reg[SPI0_REG_CS] & BIT)

/*
 * Initialize the SPI interface.  This will cause GPIO pins (7,8,9,10,11)
 * to be used for SPI pins.
 * <flags> are the required configuration of the SPI.  
 * <speed> is the required speed of the SPI bus
 */
void rpigpio_spi_init (uint32_t flags, RPIGPIO_SPI0_SPEED_t speed)
{
	// configure the GPIO pins
	gpioSetMode (SPI0_PIN_MOSI, RPIGPIO_ALT0);
	gpioSetMode (SPI0_PIN_MISO, RPIGPIO_ALT0);
	gpioSetMode (SPI0_PIN_SCLK, RPIGPIO_ALT0);
	gpioSetMode (SPI0_PIN_CS0, RPIGPIO_ALT0);
	gpioSetMode (SPI0_PIN_CS1, RPIGPIO_ALT0);
	
	// Initialize the Registers
	__spi0Reg[SPI0_REG_CS] = __spi0_cs_value = flags;
	__spi0Reg[SPI0_REG_CLK] = speed;
	
	// Clear the FIFO
	__SET_SPI0_CS(SPI0_CS_FIFOCLR);
	
	__spi0_initialized = 1;
}

/* transmit and receive <len> bytes on the SPI bus
 * <rxbuf> must have enough space of <len> bytes.
 * return 0 on sucess, -1 on failure
 */
int rpigpio_spi_send_n_receive (int len, const unsigned char * txbuf, unsigned char * rxbuf)
{
	int txcnt = 0, rxcnt = 0;
	unsigned _cont;
	
	// sanity check
	if (!__spi0_initialized || (len<=0) || !txbuf || !rxbuf)
		return -1;
	
	// Clear the FIFO and clear TA flag
	__SET_SPI0_CS(SPI0_CS_FIFOCLR | SPI0_CS_ACTIVE);

	// loop through entire len
	for (txcnt = rxcnt = 0; (txcnt<len) && (rxcnt<len); ) {
		// write as many bytes into the transmit buffer as possible
		for (; __GET_SPI0_CS(SPI0_CS_TXRDY) && (txcnt<len); txcnt++)
			__spi0Reg[SPI0_REG_FIFO] = txbuf[txcnt];
		// read as many bytes from the receive buffer as possible
		for (; __GET_SPI0_CS(SPI0_CS_RXDATA) && (rxcnt<len); rxcnt++)
			rxbuf[rxcnt] = __spi0Reg[SPI0_REG_FIFO];
	}
	// Poll the TXDONE bit
	for (; !__GET_SPI0_CS(SPI0_CS_TXDONE););
	
	// Clear the FIFO and clear TA flag
	__SET_SPI0_CS(SPI0_CS_FIFOCLR);	
	return 0;
}

/*
 * Shutdown the SPI interface.  This will cause GPIO pins (7,8,9,10,11)
 * to be configured as GPIO Input with pull-up resistor.
 */
void rpigpio_spi_fini (void)
{
	// configure the GPIO pins
	__CONFIGURE_GPIO (SPI0_PIN_MOSI, RPIGPIO_INPUT, RPIGPIO_PUD_UP);
	__CONFIGURE_GPIO (SPI0_PIN_MISO, RPIGPIO_INPUT, RPIGPIO_PUD_UP);
	__CONFIGURE_GPIO (SPI0_PIN_SCLK, RPIGPIO_INPUT, RPIGPIO_PUD_UP);
	__CONFIGURE_GPIO (SPI0_PIN_CS0, RPIGPIO_INPUT, RPIGPIO_PUD_UP);
	__CONFIGURE_GPIO (SPI0_PIN_CS1, RPIGPIO_INPUT, RPIGPIO_PUD_UP);
}

// ------------------- Test ADC -----------------------

// initialize the ADC
void ADC_init (void) 
{
	// MC3202 has a requirement of minimum 10kHz to maintain the stored charge for 1.2ms
	// we are sampling at ~48usec, and each sampling takes 3 bytes (24bits) of SPI communications
	// Hence we should set the speed at minimum 24/48e-6 = 500kHz
	// Let's play save and use 1.2MHz
	rpigpio_spi_init (0, RPIGPIO_SPI_1200kHz);
}

// get sample from ADC
uint32_t ADC_read (void)
{
	uint32_t ret = 0;
	unsigned char buf[4];
	
	// MC3202 data format
	// tx: | ST | SGL | ODD | MSB |
	// rx:                        | NUL | B11 | B10 | B9 | B8 | B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 | 
	// We set the buffer accordingly:
	//      Byte 0  -> <-           Byte 1                  -> <-             Byte 2                ->
	buf[0] = 0x3;	// 0 0 0 0 0 0 ST=1, SGL=1   = 0x03
	buf[1] = 0x40;  // ODD=0, MSB=1, 0 0 0 0 0 0 = 0x40
	buf[2] = 0x00;
	rpigpio_spi_send_n_receive (3, buf, buf);
	
    return (((buf[1] & 0x0f) << 4) | (buf[2] & 0xff));
}

// shutdown ADC
void ADC_fini (void) 
{
	rpigpio_spi_fini();
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
	
	if (gpioInitialise() < 0)
		return (-1);
		
	ADC_init();
	for (i=0; i<num; i++) {
		t = rpi_usleep(usec);
		s = ADC_read();
		printf ("tick=%010u, voltage=0x%03x\n", t, s);
	}
	
	return (0);
}

