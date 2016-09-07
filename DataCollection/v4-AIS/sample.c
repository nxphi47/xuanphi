/*
* Routines for sample structure
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sample.h"
#include "hw.h"

#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif


// convert samples in <s> to a JSON object in the output file <f>
void SAMPLE_2_json ( SAMPLE_p s, FILE * f )
{
	int i;
	
	fprintf (f, "{ \"meta\": { \"setup\": %d, \"num\": %d, \"start\": %u },\n", s->setup, s->num, s->start);
	fprintf (f, "  \"timing\": [ \n\t%6.3f", s->t_ofs[0]/1000.);
	for (i=1; i<s->num; i++) {
		if ((i % 10) == 0)
			fprintf (f, ",\n\t");
		else fprintf (f, ", ");
		fprintf (f, "%6.3f", s->t_ofs[i]/1000.);
	}
	fprintf (f, " ],\n  \"envelop\": [ \n\t%4u", s->val[0]);
	for (i=1; i<s->num; i++) {
		if ((i % 10) == 0)
			fprintf (f, ",\n\t");
		else fprintf (f, ", ");
		fprintf (f, "%4u", s->val[i]);
	}
	fprintf (f, " ]\n}");
}

static int __fake_len = 0;
static int __fake_ADC[] = {
        942,  947,  944,  942,  942,  943,  942,  942,  942,  943,
        943,  942,  942,  942,  943,  943,  942,  942,  942,  941,
        943,  942,  942,  942,  941,  942,  943,  943,  941,  943,
        942,  943,  942,  942,  942,  942,  942,  942,  943,  943,
        942,  942,  3851,  2953,  932,  932,  1048,  1067,  2223,  932,
        932,  932,  932,  932,  932,  932,  932,  932,  1474,  2888,
        2932,  2240,  1224,  1473,  1558,  1538,  1489,  1415,  1319,  1262,
        1246,  1204,  1151,  1123,  1113,  1093,  1065,  1049,  1039,  1031,
        1016,  1006,  999,  993,  986,  981,  975,  974,  968,  968,
        968,  965,  961,  955,  955,  949,  951,  950,  953,  951,
        950,  950,  951,  951,  949,  949,  948,  947,  946,  947,
        948,  946,  948,  948,  944,  946,  944,  946,  944,  946,
        944,  947,  944,  943,  943,  942,  943,  944,  943,  942,
        943,  943,  943,  943,  943,  942,  943,  942,  942,  943,
        943,  942,  942,  942,  943,  948,  945,  943,  957,  969,
        971,  938,  934,  937,  938,  940,  941,  945,  951,  954,
        954,  934,  937,  939,  940,  943,  943,  944,  941,  942,
        943,  942,  942,  943,  943,  942,  944,  943,  943,  944,
        943,  943,  941,  942,  944,  945,  942,  943,  942,  942,
        942,  942,  942,  941,  942,  943,  943,  942,  942,  942,
        942,  943,  942,  942,  942,  942,  942,  943,  943,  945,
        943,  944,  942,  943,  944,  943,  942,  942,  942,  943,
        943,  942,  944,  944,  943,  942,  941,  942,  942,  942,
        941,  942,  942,  944,  941,  941,  942,  942,  942,  942,
        942,  942,  942,  941,  941,  942,  941,  941,  941,  941,
        943,  942,  943,  943,  942,  943,  942,  942,  941,  942,
        943,  943,  942,  942,  941,  942,  942,  943,  942,  943,
        942,  942,  943,  943,  943,  942,  941,  941,  943,  942,
        -1 };
	
static SAMPLE_t __sample;

// Fill in fake values in the sample <s>
// <num> specifies number of samples to generate
// <tusec> specifies the minimum time interval between samples
SAMPLE_p SAMPLE_gen_fake (SAMPLE_p s, int num, uint32_t tusec)
{
	uint32_t t, v;
	int i;
	
	if (!s)
		s = &__sample;

	if (__fake_len <= 0) {
		srand((unsigned int)time(NULL));
		for (__fake_len=0; __fake_ADC[__fake_len] > 0; __fake_len++);
	}
	
	for (i=0, t=0; i<num; i++) {
		// get v
		if (i < __fake_len)
			v = __fake_ADC[i];
		v += (v < 1100) ? (rand() & 0x07) - (rand() & 0x07) : (rand() & 0x1f) - (rand() & 0x1f);
		s->val[i] = v;
		// update t
		s->t_ofs[i] = t;
		t += tusec + (((rand() & 0x07) > 5) ? rand() & 0x3f : 0);
	}
	s->num = num;
	
	return (s);
}

// ----------------------------- CAPTURE -------------------------------------

// Here, we implement software-level peak detection
#define __PEAK_WINDOW	5
static uint32_t __idle = 0;						// idle voltage
static uint32_t __window[__PEAK_WINDOW];		// voltage in each window
static uint32_t __curr_index = 0;				// index of current window
static uint32_t __window_ts = 0;				// timestamp of peak detection window

// Idle voltage detection for the specified time
// this will reset the peak window detection algorithm as well
static uint32_t __idle_detect(uint32_t tusec)
{
	uint32_t n, t;
	
	// initial idle detection
	t = HW_tick();
	for (n=__curr_index=0, __idle=0; n<5; n++) {
		__idle += (__window[__curr_index] = ADC_read());
		// check for wrap around
		if (++__curr_index >= __PEAK_WINDOW)
			__curr_index = 0;
	}
	
	// let the processor sleep
	tusec -= HW_tick() - t;
	t = HW_tick();
	if (tusec > 300)
		usleep(tusec-200);
		
	// final idle detection
	tusec -= HW_tick() - t;
	for (t=HW_tick(), __curr_index=0; (__window_ts = HW_tick()) - t < tusec; n++) {
		__idle += (__window[__curr_index] = ADC_read());
		// check for wrap around
		if (++__curr_index >= __PEAK_WINDOW)
			__curr_index = 0;
	}
	return (__idle /= n);
}

// Get a peak sample
static uint32_t __get_peak (uint32_t period)
{
	uint32_t t = HW_tick();
	uint32_t n = __PEAK_WINDOW;
	uint32_t i, peak = (__idle > 10)?(__idle-10):0;
	
	// check if window is valid
	if (t - __window_ts > __PEAK_WINDOW * ADC_READ_TIME)
		__curr_index = n = 0;
	// start reading in values for the given period
	for (; (__window_ts = HW_tick()) - t < period; n++) {
		__window[__curr_index] = ADC_read();
		// check for wrap around
		if (++__curr_index >= __PEAK_WINDOW)
			__curr_index = 0;
	}
	// get max value
	if (n > __PEAK_WINDOW)
		n = __PEAK_WINDOW;
	for (i=0; i<n; i++)
		if (__window[i] > peak)
			peak = __window[i];
	return (peak);
}

// Capture one sample <s>
// setup: the measurement set up to use
// n_samples: number of samples to collect
// t_sample: usec between sample time
// t_wait: usec to wait between firing and sampling 
SAMPLE_p SAMPLE_capture ( SAMPLE_p s, int setup, int n_samples, int t_sample, int t_wait )
{
#ifdef __USE_ORIGINAL_PEAK_DETECTION__
	uint32_t t, v, vmax, idleLevel;
	int n;
#endif
	uint32_t t_start;
	int i;
	
	// sanity check
	if (n_samples > MAX_SAMPLE_PTS)
		n_samples = MAX_SAMPLE_PTS;
	if (t_wait < 0)
		t_wait = 15000; // 15 msec
	if (!s)
		s = &__sample;
	
	// first fire the sensor
	debug("\n--- firing the sensor and will sample at (1/%dusec) sampling rate after %.2f msec ... \n", t_sample, t_wait/1000.);
	LIN_send_StartMeasurement(setup);

#ifdef __USE_ORIGINAL_PEAK_DETECTION__
	// we made use of the wait time to sense the minimum voltage
	for (t_start = HW_tick(), idleLevel=0, n=0; HW_tick() - t_start < t_wait - ADC_READ_TIME; n++) 
		idleLevel += ADC_read();
	if (n>0)
		idleLevel /= n;
	t_start = HW_tick();
	// Collect the samples
	for (i=0; i<n_samples; i++) {
		// we collect multiple samples within each sampling interval 
		// and take the max -- aka peak
		vmax = (idleLevel > 10) ? (idleLevel -10) : 0;
		for (t=HW_tick(); HW_tick() - t < t_sample; ) {
			v = ADC_read();
			if (v > vmax)
				vmax = v;
		}
		s->t_ofs[i] = t - t_start;
		s->val[i] = vmax;
	}
#else // __USE_NEW_PEAK_DETECTION__
	__idle_detect(t_wait - ADC_READ_TIME);
	t_start = HW_tick();
	// Collect the samples
	for (i=0; i<n_samples; i++) {
		s->t_ofs[i] = HW_tick() - t_start;
		s->val[i] = __get_peak(t_sample - ADC_READ_TIME_HALF);
	}
#endif	// __USE_PEAK_DETECTION__
	s->num = i;
	s->setup = setup;
	return (s);
}

