/*
 * Realtime Capturing of AIS sensor waveform
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/select.h>
#include <string.h>

#include "sample.h"
#include "hw.h"
#include "comms.h"

#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif

// -------- INTERNAL VARIABLES ----------------

// each configuration parameter
typedef struct __cfg_param__ {
	char name[32];			// name of parameter
	int val;				// value of the parameter
	int min;				// minimum possible value
	int max;				// maximum possible value
} CFGPARAM_t, *CFGPARAM_p;

// Configurations
static CFGPARAM_t __cfg[] = {
	{ name: "SETUP",     val: 0,     min: 0,    max: 3 },				// measurement setup # to use
	{ name: "T-BURST",   val: 1000,  min: 30,   max: 2000 },			// time between burst (msec)
	{ name: "T-SAMPLE",  val: 50,    min: 20,   max: 1000 },			// time between sample (usec)
	{ name: "N-SAMPLES", val: 400,   min: 100,  max: MAX_SAMPLE_PTS },	// number of samples per burst
	{ name: "T-WAIT",    val: 1500,  min: 100,  max: 20000 },			// time to wait after burst b4 sampling starts (usec)
};

#define CFG_SETUP		__cfg[0].val
#define CFG_T_BURST		__cfg[1].val
#define CFG_T_SAMPLE	__cfg[2].val
#define CFG_N_SAMPLES	__cfg[3].val
#define CFG_T_WAIT		__cfg[4].val
#define NUM_CFG_PARAM	5

static uint32_t __ctrlflags = 0x00;
#define CTRL_FAKE_SAMPLING	0x0010
#define CTRL_QUIT			0x8000
#define CTRL_RANGING		0x0001

static uint32_t __last_burst = 0;

// ----------------------------- COMMS CALLBACK -------------------------------------
	
// call back for "QUIT"
static void handle_quit (void * ignore, const char * word, char * msg)
{
	debug ("** 'QUIT' received.\n");
	__ctrlflags |= CTRL_QUIT;
	COMMS_set_flag(COMMS_FLAG_QUIT);
}

// call back for "SETCFG"
static void handle_setcfg (void * ignore, const char * word, char * msg)
{
	int val, c;
	char *p, *ep;
	
	debug ("** 'SETCFG' received: %s\n", msg);
	while (msg) {
		// skip continuous spaces
		while (*msg == ' ')
			msg++;
		// get the equal sign
		p = strchr(msg, '=');
		if (!p) // no '=', stop parsing
			return;
		// get the value after '='
		ep = strchr(p+1, ' ');
		if (ep)
			*ep = 0;
		val = atoi(p+1);
		if (ep)
			*ep = ' ';
		// determine which value to set
		for (c=0; c<NUM_CFG_PARAM; c++) {
			if (strncmp(__cfg[c].name, msg, strlen(__cfg[c].name)) == 0) {
				if ((val >= __cfg[c].min) && (val <= __cfg[c].max)) {
					__cfg[c].val = val;
					debug("handle_setcfg(): setting %s to %d\n", __cfg[c].name, __cfg[c].val);
				} else
					debug("handle_setcfg(): invalid value (%d) for %s\n", val, __cfg[c].name);
				break;
			}
		}
		msg = ep;
	}
}

// callback for "GETCFG"
static void handle_getcfg (void * ignore, const char * word, char * msg)
{
	char buf[__BUF_SIZE__];
	int n, c;
	
	// return configuration params
	for (c=n=0; c<NUM_CFG_PARAM; c++)
		n += sprintf (buf+n, "%s=%d ", __cfg[c].name, __cfg[c].val);
	COMMS_send_msg("CONFIG", buf);
	debug("handle_getcfg(): sending back '%s'\n", buf);
}

// callback for "GETSTATUS"
static void handle_getstatus (void * ignore, const char * word, char * msg)
{
	char buf[__BUF_SIZE__];
	int n;
	
	// return hw status
	n = sprintf (buf, "HW=0x%x ", hw_state);
	n += sprintf (buf+n, "SEN=%s ", (hw_state & HW_NO_SENSOR)?"NO":"OK");
	n += sprintf (buf+n, "12V=%s ", (hw_state & HW_NO_12V_SUPPLY)?"NO":"OK");
	n += sprintf (buf+n, "LIN-BRK=%s ", (hw_state & HW_LIN_BREAK_HIGH)?"HIGH":"OK");
	n += sprintf (buf+n, "LIN-RX=%s ", (hw_state & HW_LIN_RX_LOW)?"LOW":"OK");
	n += sprintf (buf+n, "ADC=%s ", (hw_state & HW_ADC_HIGH)?"HIGH":((hw_state & HW_ADC_LOW)?"LOW":"OK"));
	COMMS_send_msg("STATUS", buf);
	debug("handle_getstatus(): sending back '%s'\n", buf);
}

// callback for "START"
static void handle_start (void * ignore, const char * word, char * msg)
{
	debug ("** 'START' received\n");

	// set flag to start ranging
	__ctrlflags |= CTRL_RANGING;
	// request comms main loop to use short timeout
	COMMS_set_flag(COMMS_FLAG_SHORT_TIMEOUT);
	// Turn on LED
	LED_set_state(LED_BURST, LED_STATE_ON);
	// clear last burst time
	__last_burst = HW_tick() - CFG_T_BURST * 1000;
}	

// callback for "STOP"
static void handle_stop (void * ignore, const char * word, char * msg)
{
	debug ("** 'STOP' received\n");

	// set flag to stop ranging
	__ctrlflags &= ~CTRL_RANGING;
	// request comms main loop to use long timeout
	COMMS_clr_flag(COMMS_FLAG_SHORT_TIMEOUT);
	// Turn off LED
	LED_set_state(LED_BURST, LED_STATE_OFF);
}

// Periodic Callback
static void periodic_callback (void * ignore, const char * ignore1, char * ignore2)
{
	SAMPLE_p sample;
#ifdef __DEBUG__
	uint32_t ts;
#endif
	// let HW module do its test
	HW_check();
	
	// let LED module do its blinking trick
	LED_timeout();

	// check Ranging
	if (__ctrlflags & CTRL_RANGING) {
		// check if it is time to do next ranging
		if (HW_tick() - __last_burst <= CFG_T_BURST * 1000)
			return;
		// perform ranging
		__last_burst = HW_tick();
		if (__ctrlflags & CTRL_FAKE_SAMPLING) {
			// use fake sampling
			debug("Generating fake sample ...\n");
			sample = SAMPLE_gen_fake(NULL, CFG_N_SAMPLES, CFG_T_SAMPLE);
		} else { 
			// actual capturing
			sample = SAMPLE_capture(NULL, CFG_SETUP, CFG_N_SAMPLES, CFG_T_SAMPLE, CFG_T_WAIT);
		}
		sample->start = __last_burst;
#ifdef __DEBUG__
		SAMPLE_2_json(sample, stdout);
		ts = HW_tick();
#endif
		COMMS_send_sample(sample);
#ifdef __DEBUG__
		debug("It takes %d usec to send a sample\n", HW_tick() - ts);
#endif
	} else {
		// nothing to do
		return;
	}
}

#ifdef __DEBUG__
static void handle_hwdbg_on (void * ignore, const char * word,  char * msg)
{
	debug("*** Entering HW-DEBUG mode ...\n");
	HW_DBG_init();
	COMMS_send_msg("HW-DBG %s\0", HW_DBG_show());
}

static void handle_hwdbg_off (void * ignore, const char * word,  char * msg)
{
	debug("*** Exiting from HW-DEBUG mode ...\n");
	COMMS_send_msg("HW-DBG %s\0", HW_DBG_show());
	HW_DBG_close();
}

static void handle_hwdbg_set (void * ignore, const char * word,  char * msg)
{
	debug("*** HW-DBG-SET: %s ...\n", msg);
	HW_DBG_set(msg);
	COMMS_send_msg("HW-DBG %s\0", HW_DBG_show());
}

#endif // __DEBUG__

// ---------------------------- MAIN -------------------------------

int main (int argc, char * argv[])
{
	int i;
	
	// parse input argument
	for (i=1; i<argc; i++) {
		if (strcmp(argv[i], "-F") == 0) {
			// fake sampling
			__ctrlflags |= CTRL_FAKE_SAMPLING;
			i++;
			continue;
		}
	}
	
	// main initialization
	if (HW_init() < 0)
		return (-1);
	COMMS_init(0);
	
	// setup callbacks
	COMMS_set_callback("QUIT", handle_quit, NULL);
	COMMS_set_callback("SETCFG", handle_setcfg, NULL);
	COMMS_set_callback("GETCFG", handle_getcfg, NULL);
	COMMS_set_callback("GETSTATUS", handle_getstatus, NULL);
	COMMS_set_callback("START", handle_start, NULL);
	COMMS_set_callback("STOP", handle_stop, NULL);
#ifdef __DEBUG__
	COMMS_set_callback("HWDBG-ON", handle_hwdbg_on, NULL);
	COMMS_set_callback("HWDBG-OFF", handle_hwdbg_off, NULL);
	COMMS_set_callback("HWDBG-SET", handle_hwdbg_set, NULL);
#endif

	// main loop
	COMMS_main_loop(periodic_callback, NULL);
	
	// clean up
	COMMS_close();
	HW_close();
	return (0);
}
