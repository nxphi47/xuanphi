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
	int sensorid;				// triggerred by which sensor
	SAMPLE_p samples;			// samples array
	SHISTORY_p next;			// next history
	int histid;					// history ID
} SHISTORY_t;

#define MAX_SAMPLES		2048
#define NUM_SENSORS		4

// Structure for data collection
typedef struct __tag_sensor_control__ *SENSOR_p;
typedef struct __tag_sensor_control__ {
	int sensorid;				// index of this sensor
	uint32_t A0A1_bits_set;		// address bits to set
	uint32_t A0A1_bits_clr;		// address bits to clear
	uint32_t usec_last_range;	// timestamp of last time ranging was started
	int enabled;				// enabled or disabled
	SHISTORY_p curr;			// current samples
	SHISTORY_p prev;			// previous history of samples
} SENSOR_t;

// ctrl flags
#define RPICB_ENABLE_CAM	0x01

// Default Minimum callback time
#define RPICB_MIN_CALLBACK	500		// 500 usec minimum

// structure for overall rpi control block
typedef struct __tag_rpi_control__ *RPI_CB_p;
typedef struct __tag_rpi_control__ {
	uint32_t usec_ranging;		// ranging time (usec)
	uint32_t usec_sampling;		// sampling period (usec)
	uint32_t usec_next;			// time (usec) to wait before next sensor can range
	uint32_t usec_cycle;		// cycle time (usec): time to wait before same sensor can range
	uint32_t usec_sample;		// time between samples (usec)
	uint32_t ctrl_flags;		// control flags
	uint32_t idle_cb_usec;		// minimum amount of idle time (usec) before callback can be triggerred
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

/* callback type during sensor idle period
 * The first argument is a user specified callback data (typecasted to void*)
 * The second argument is the <last> time in usec
 * The third argument is the <period> time in usec
 * The last argument is the sensor pointer
 * The callback function should use the 2nd and 3rd arguments to call 
 * rpigpio_time_remaining() to get the amount of idle time remaining in usec.
 * When it is close to 0, the callback should return immediately
 */
typedef void (*IDLECALLBACK_f)(void *, uint32_t, uint32_t, SENSOR_p);

/* Start a cycle of ranging and sampling by sensor <sen>
 * Returns a pointer to the collected sample history
 * NULL on error (not enabled, or <sen> is NULL)
 * This function allows user to set a callback function whenever the ranging cycle is idle (waiting for timeout)
 */
extern SHISTORY_p rpigpio_sensor_range_and_sample_cb (SENSOR_p sen, IDLECALLBACK_f idle, void * cb_data);

/* Start a cycle of ranging and sampling by sensor <sen>
 * Returns a pointer to the collected sample history
 * NULL on error (not enabled, or <sen> is NULL)
 */
extern SHISTORY_p rpigpio_sensor_range_and_sample (SENSOR_p sen);

/* Start a cycle of ranging for sensir <sen> 
 * and collect samples from ALL sensors (regardless if enabled)
 * Returns -1 on error (not enabled, or <sen> is NULL)
 * The readings are stored in each sensor's curr pointer
 */
extern int rpigpio_sensor_range_and_sample_all (SENSOR_p sen);
extern int rpigpio_sensor_range_and_sample_all_not_workingf (SENSOR_p sen);

/* Move current history to previous.
 * If <max> is greater than 1, maximum number of samples history in previous
 * will be <max>.  Older histories will be removed.
 */
extern void rpigpio_sensor_update_history (SENSOR_p sen, int max);

/* Move current history to previous, and set the history ID
 * If <max> is greater than 1, maximum number of samples history in previous
 * will be <max>.  Older histories will be removed.
 */
extern void rpigpio_sensor_update_history_with_id (SENSOR_p sen, int max, int histid);

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
extern const char * rpigpio_history_print (SHISTORY_p hist, int senid);

/* ------------------------------------
 * Definitions in rpi-cam.c
 * ------------------------------------
 */
 
// Initialize the Camera Module
extern int rpicam_init (void);

// Capture an image
extern void rpicam_capture (void);

// Save the previously captured image to the specified filename with the specified index
// Resulting filename of the image will be '<fname>-<histid>.png'
extern void rpicam_save_image ( const char * fname, int histid );

// Clean up the camera module
extern void rpicam_fini (void);

#endif // __RPIGPIO__
