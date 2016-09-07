#ifndef __HW_H__
#define __HW_H__

// for typedef of uint32_t
#include <stdint.h>


// initializes the LED, LIN and ADC interface
// function calls bcm2835_init(), LIN_init(), LED_init() and ADC_init()
extern int HW_init (void);

// closes the LED, LIN and ADC interface
// function calls ADC_fini(), LIN_close(), LED_close() and bcm2835_close()
extern void HW_close (void);

// Get the current mircoseconds tick
extern uint32_t HW_tick (void);

// Sleep for specified microseconds
extern uint32_t HW_usleep (uint32_t usec);

/* Initialize the LIN interface
 *   returns 0 on success
 */
extern int LIN_init (void);

/* Close the LIN interface
 */
void LIN_close (void);

/* Fire the AIS sensor by sending an LIN StartMeasurement frame */
extern void LIN_send_StartMeasurement (int measurementSetup);

// each ADC reading requires 3 bytes of tx/rx at 1.953MHz clock
// hence each reading takes around 12usecs
#define ADC_READ_TIME		12
#define ADC_READ_TIME_HALF	6

// initialize the ADC
extern void ADC_init (void);

// get sample from ADC
extern uint32_t ADC_read (void);

// shutdown ADC
extern void ADC_fini (void);

/*
 * LED support
 */
extern void * LED_POWER;
extern void * LED_BURST;
extern void * LED_LINCOMMS;

#define LED_STATE_OFF			0x0100		// off state
#define LED_STATE_SLOW_BLINK	0x0200		// slow blinking
#define LED_STATE_FAST_BLINK	0x0400		// fast blinking
#define LED_STATE_ON			0x0800		// turned on

// initialization
extern void LED_init (void);

// set the state of the specified LED using one of the define states:
//		ON, OFF, SLOW_BLINK, FAST_BLINK
extern void LED_set_state ( void * led, uint32_t state );

// periodic callback
extern void LED_timeout ( void );

// close the LEDs
extern void LED_close (void);

// Hwardware check
//		contains one of the following flags if abnormal
extern uint32_t hw_state;

// ADC detects an extremely low voltage ==> probably 12V not connected
#define HW_ADC_LOW			0x0001

// ADC detects an extremely high voltage ==> probably sensor not connected
#define HW_ADC_HIGH			0x0002

// LIN is at default LOW ==> probably 12V not connected
#define HW_LIN_RX_LOW		0x0010

// LIN is at HIGH when break is sent ==> probably 12V not connected
#define HW_LIN_BREAK_HIGH	0x0020

// Estimated 12V not connected
#define HW_NO_12V_SUPPLY	0x0100

// Estimated sensor not connected
#define HW_NO_SENSOR		0x0200

// Check the current status of hardware
//		The function will restrict itself to check once every 5 secs max
extern uint32_t HW_check (void);

#ifdef __DEBUG__

// initialize the hw debugger
extern void HW_DBG_init (void);

// close the hw debugger
extern void HW_DBG_close (void);

// show the current pin status
extern const char * HW_DBG_show (void);

// set the pins levels
extern void HW_DBG_set ( const char * str );

#endif // __DEBUG__

#endif // __HW_H__
