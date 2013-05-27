#ifndef __OSC_H
#define __OSC_H

#include "types.h"


typedef enum {
	OSC_TYPE_SINE = 0,
	OSC_TYPE_SQUARE = 1,
	OSC_TYPE_TRIANGLE = 2,
	OSC_TYPE_SAWTOOTH = 4,
	OSC_TYPE_NOISE_WHITE = 5,
	OSC_TYPE_WAVE = 10
} osc_type_t;

typedef struct {
	osc_type_t	type;
	scalar_t	phase;
	scalar_t	phase_offset;
	scalar_t	freq;
	scalar_t *	waveform;
	scalar_t	waveform_length;
} osc_t;


scalar_t osc_fetch_sample(osc_t * osc, scalar_t sample_freq);
void osc_init(osc_t * osc, osc_type_t type);

#endif
