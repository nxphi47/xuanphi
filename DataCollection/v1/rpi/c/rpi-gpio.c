/*
	Convenient APIs for Data Collection Circuit
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "rpi-gpio.h"

#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif


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
#define RPIGPIO_T_SAMPLING_USEC	23000	// 23msec
#define RPIGPIO_T_NEXT_USEC		50000	// 50msec
#define RPIGPIO_T_CYCLE_USEC	100000	// 100msec


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
		
		for (i=0; i<NUM_SENSORS; i++)
			rpigpio_sensor_init(__rpicb->sensors+i, i);
			
		__rpistr = (char*)calloc(MAX_SAMPLES*18+120, sizeof(char));
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
	p += sprintf(__rpistr, "{ 'sensors': [ %d, %d, %d, %d ],\n", 
		__rpicb->sensors[0].enabled, __rpicb->sensors[1].enabled, 
		__rpicb->sensors[2].enabled, __rpicb->sensors[3].enabled);
	p += sprintf(p, "  'T-ranging': %d, 'T-samping': %d, 'T-next': %d, 'T-cycle': %d }\n",
		__rpicb->usec_ranging, __rpicb->usec_sampling,
		__rpicb->usec_next, __rpicb->usec_cycle);
	return (__rpistr);
}

/* --------------------------------------------------------------------
 *  Common GPIO utilities
 * --------------------------------------------------------------------
 */

#ifdef __VER_2_CIRCUIT__

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

/* Read data
 *  It is assumed that the address pins A0 & A1 are already set prior to this call
 *	For version 2, 8-bit data will be returned
 * 	For version 3, 16-bit data will be returned
 */
SAMPLE rpigpio_read_data (void)
{
	uint32_t res=0;

#ifdef __VER_2_CIRCUIT__
	__latch__();
	usleep(1);			// ensure enough time for data to appear on databus
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
int rpigpio_time_remaining (uint32_t last, uint32_t period) 
{
	uint32_t now = gpioTick();
	
	//printf("-- time_remaining(): now=%d, last=%d, period=%d\n", now, last, period);
	if (now < last) {
		// Overflow in timing
		now &= 0xfffffff;
		last &= 0xfffffff; 
		now += 0x10000000;
	}
	now -= last;
	return ((now < period) ? period - now : 0);
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

/* Start a cycle of ranging and sampling by sensor <sen>
 * Returns a pointer to the collected sample history
 * NULL on error (not enabled, or <sen> is NULL)
 */
SHISTORY_p rpigpio_sensor_range_and_sample (SENSOR_p sen)
{
	SHISTORY_p p;
	
	if (sen && sen->enabled) {
		// ensure T_cycle is over before ranging
		int tdiff = rpigpio_time_remaining(sen->usec_last_range, __rpicb->usec_cycle);
		debug("---- %s: cycle-wait: tdiff=%d\n", __func__, tdiff);
		if (tdiff > 1)
			usleep(tdiff-1);
		// initialize address bus
		gpioSetBank1(sen->A0A1_bits_set | RPIGPIO_ALE_bit);
		gpioClearBank1(sen->A0A1_bits_clr);
		printf("---- %s: going to range\n", __func__);
		sen->usec_last_range = gpioTick();
		__range__();
		p = rpigpio_history_init (sen->curr, MAX_SAMPLES);
		__convert__(); // activate ADC
		// wait for T_range to be over
		tdiff = rpigpio_time_remaining(sen->usec_last_range, __rpicb->usec_ranging);
		debug("---- %s: range-wait: tdiff=%d\n", __func__, tdiff);
		if (tdiff > 10)
			usleep(tdiff);
		// collect data
		p->sec_start_time = (uint32_t) time();
		p->usec_start_time = gpioTick();
		while (rpigpio_time_remaining(p->usec_start_time, __rpicb->usec_sampling) > 0) {
			p->samples[p->num_samples].timestamp = gpioTick();
			__convert__();
			usleep(5); // 4.5 usec max conversion time
			p->samples[p->num_samples++].value = rpigpio_read_data();
			if (p->num_samples >= MAX_SAMPLES)
				break;
		}
		debug("---- %s: returning\n", __func__);
#ifdef __arm__
		gpioWrite(RPIGPIO_ALE, 1);	// disable ALE
#endif
		return (sen->curr = p);
	}
	return (NULL);
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
const char * rpigpio_history_print (SHISTORY_p hist)
{
	char * p = __rpistr;
	unsigned pt;
	int i, tdiff;
	
	if (!hist || !hist->num_samples)
		return (NULL);
		
	p += sprintf (p, "{  'start': [%u, %u], 'len': %d,\n  'samples': [", 
			hist->sec_start_time, hist->usec_start_time % 1000000, hist->num_samples);
	
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
