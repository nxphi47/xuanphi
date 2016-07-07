#ifndef __RPIGPIO_H__
#define __RPIGPIO_H__

#include <stdint.h>

/* ------------------------------------
 * Definitions in rpi-gpio.c
 * ------------------------------------
 */
 
#ifdef __VER_2_CIRCUIT__
typedef uint8_t SAMPLE;
#else
typedef uint16_t SAMPLE;
#endif

// Structure for sample
typedef struct __tag_sample__ *SAMPLE_p;
typedef struct __tag_sample__ {
	uint32_t timestamp;			// timestamp
	SAMPLE value;				// value of sampled voltage
} SAMPLE_t;

// Structure to store history
typedef struct __tag_sample_hist__ * SHISTORY_p;
typedef struct __tag_sample_hist__ {
	uint32_t sec_start_time;	// start time of history (secs since epoch)
	uint32_t usec_start_time;	// start time of history (usec)
	int num_samples;			// number of samples in this history
	SAMPLE_p samples;			// samples array
	SHISTORY_p next;			// next history
} SHISTORY_t;

#define MAX_SAMPLES		1024
#define NUM_SENSORS		4

// Structure for data collection
typedef struct __tag_sensor_control__ *SENSOR_p;
typedef struct __tag_sensor_control__ {
	uint32_t A0A1_bits_set;		// address bits to set
	uint32_t A0A1_bits_clr;		// address bits to clear
	uint32_t usec_last_range;	// timestamp of last time ranging was started
	int enabled;				// enabled or disabled
	SHISTORY_p curr;			// current samples
	SHISTORY_p prev;			// previous history of samples
} SENSOR_t;

// structure for overall rpi control block
typedef struct __tag_rpi_control__ *RPI_CB_p;
typedef struct __tag_rpi_control__ {
	uint32_t usec_ranging;		// ranging time (usec)
	uint32_t usec_sampling;		// sampling time (usec)
	uint32_t usec_next;			// time (usec) to wait before next sensor can range
	uint32_t usec_cycle;		// cycle time (usec): time to wait before same sensor can range
	SENSOR_t sensors[NUM_SENSORS];
} RPI_CB_t;

/* RPIGPIO control block initialization
 * 	returns pointer to a new control block on successful initialization, 
 * 	NULL on error.
 */
extern RPI_CB_p rpigpio_init (void);

/* Cleanup the RPiGPIO
 */
extern void rpigpio_fini (void);

/* Print information on RPi in JSON format
 * Calling function should not modify or deallocate the returned string
 * and should treat the returned string as only valid until the next call
 * to this function or to rpigpio_history_print().
 */
extern const char *  rpigpio_print (void);

/* start ranging for sensor <id> */
extern void rpigpio_start_range (int id);

/* start conversion for sensor <id> */
extern void rpigpio_start_convert (int id);

/* start latching for sensor <id> */
extern void rpigpio_start_latch (int id);

/* Read data
 *  It is assumed that the address pins A0 & A1 are already set prior to this call
 *	For version 2, 8-bit data will be returned
 * 	For version 3, 16-bit data will be returned
 */
extern SAMPLE rpigpio_read_data (void);

/* compare timing:
 *	Return 0 if <period> is exceeded since <last>, remaining usec otherwise
 *  <last> and <period> are in usec
 */
extern int rpigpio_time_remaining (uint32_t last, uint32_t period);

/*
 * Debug GPIO pins
 */
extern void rpigpio_pins_show (void);

/* Initialize a sensor control block <sen>
 * If <sen> is NULL, a new control block will be allocated
 * <id> is the id of the sensor, to determine the address signals
 * Return <sen> on sucess
 */
extern SENSOR_p rpigpio_sensor_init (SENSOR_p sen, int id);

/* Release resources held by sensor <sen>
 * Note that this only dellocates sample history buffer.
 * It does not deallocate the sensor control block itself
 */
extern void rpigpio_sensor_fini (SENSOR_p sen);

/* Start a cycle of ranging and sampling by sensor <sen>
 * Returns a pointer to the collected sample history
 * NULL on error (not enabled, or <sen> is NULL)
 */
extern SHISTORY_p rpigpio_sensor_range_and_sample (SENSOR_p sen);

/* Move current history to previous.
 * If <max> is greater than 1, maximum number of samples history in previous
 * will be <max>.  Older histories will be removed.
 */
extern void rpigpio_sensor_update_history (SENSOR_p sen, int max);

/* Initialize a sample history <hist>
 * If <hist> is NULL, a new sample history will be allocated
 * If <size> is <=0, no new sample buffer will be allocated
 * Return <hist> on sucess
 */
extern SHISTORY_p rpigpio_history_init (SHISTORY_p hist, int size);

/* Deallocates a sample history <hist>
 * The next pointer is ignored.
 */
extern void rpigpio_history_free (SHISTORY_p hist);

/* prints the samples to a string in JSON format
 * Calling function should not modify or deallocate the returned string
 * and should treat the returned string as only valid until the next call
 * to this function.
 */
extern const char * rpigpio_history_print (SHISTORY_p hist);

/* ------------------------------------
 * Definitions in minimal_gpio.c
 * ------------------------------------
 */

/* set the mode of GPIO pin. */
extern void gpioSetMode(unsigned gpio, unsigned mode);

/* definition of modes are: */

#define RPIGPIO_INPUT  0
#define RPIGPIO_OUTPUT 1
#define RPIGPIO_ALT0   4
#define RPIGPIO_ALT1   5
#define RPIGPIO_ALT2   6
#define RPIGPIO_ALT3   7
#define RPIGPIO_ALT4   3
#define RPIGPIO_ALT5   2

/* get the current mode of specified GPIO pin */
extern int gpioGetMode(unsigned gpio);

/* Configure pull-up-down of specified GPIO pin */
extern void gpioSetPullUpDown(unsigned gpio, unsigned pud);

/* definition of pull-up, pull-down are: */
#define RPIGPIO_PUD_OFF  0
#define RPIGPIO_PUD_DOWN 1
#define RPIGPIO_PUD_UP   2

/* read the value (0=LOW, 1=HIGH) of specified GPIO pin */
extern int gpioRead(unsigned gpio);

/* set the value (0=LOW, 1=HIGH) of specified GPIO pin */
extern void gpioWrite(unsigned gpio, unsigned level);

/* Send a pulse of specified length (usec) to the specified GPIO pin 
   Use level to determine a high (level=1) or low (level=0 pulse */
extern void gpioTrigger(unsigned gpio, unsigned pulseLen, unsigned level);

/* Read in GPIO Bank #1 -- containing GPIO pins 0-31 */
extern uint32_t gpioReadBank1(void);

/* Read in GPIO Bank #2 -- containing GPIO pins 32-54 */
extern uint32_t gpioReadBank2(void);

/* Set the specified pins to LOW for GPIO pins 0-31
   To specify GPIO pin X, the X bit in <bits> should be 1 */
extern void gpioClearBank1(uint32_t bits);

/* Set the specified pins to LOW for GPIO pins 32-54
   To specify GPIO pin X, the (X-32) bit in <bits> should be 1 */
extern void gpioClearBank2(uint32_t bits);

/* Set the specified pins to HIGH for GPIO pins 0-31
   To specify GPIO pin X, the X bit in <bits> should be 1 */
extern void gpioSetBank1(uint32_t bits);

/* Set the specified pins to HIGH for GPIO pins 32-54
   To specify GPIO pin X, the (X-32) bit in <bits> should be 1 */
extern void gpioSetBank2(uint32_t bits);

/* Returns the hardware revision number
	2 or 3: 			26-pin header (P1)
	4, 5, 6, or 15:		26-pin header (P1) + 8-pin header (P5)
	16 or greater:		40-pin header (J8)
*/
extern unsigned gpioHardwareRevision(void);

/* Returns the number of microseconds after system boot. Wraps around
   after 1 hour 11 minutes 35 seconds. */
extern uint32_t gpioTick(void);

/* Initialize the mapping.  MUST be called!!! */
extern int gpioInitialise(void);

#endif // __RPIGPIO__
