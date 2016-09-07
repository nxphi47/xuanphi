#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include <stdint.h>
#include <stdio.h>

// maximum number of sampling points
#define MAX_SAMPLE_PTS	1024

// structure to record the captured sample
typedef struct __sample_t__ {
	uint32_t t_ofs[MAX_SAMPLE_PTS];	// time offset from 1st sample
	uint32_t val[MAX_SAMPLE_PTS];	// sampled voltage
	uint32_t num;					// number of samples
	int setup;						// the Setup # used in AIS sensor
	uint32_t start;					// start capturing time
} SAMPLE_t, * SAMPLE_p;

// convert samples in <s> to a JSON object in the output file <f>
extern void SAMPLE_2_json ( SAMPLE_p s, FILE * f );

// Fill in fake values in the sample <s>
// <num> specifies number of samples to generate
// <tusec> specifies the minimum time interval between samples
extern SAMPLE_p SAMPLE_gen_fake (SAMPLE_p s, int num, uint32_t tusec);

// Capture one sample <s>
// setup: the measurement set up to use
// n_samples: number of samples to collect
// t_sample: usec between sample time
// t_wait: usec to wait between firing and sampling 
extern SAMPLE_p SAMPLE_capture ( SAMPLE_p s, int setup, int n_samples, int t_sample, int t_wait );


#endif // __SAMPLE_H__
