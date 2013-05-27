#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include "osc.h"

void osc_init(osc_t * osc, osc_type_t type) 
{
	osc->type = type;
	osc->freq = 120;
	osc->phase_offset = 0;

	switch (osc->type) {
	case OSC_TYPE_SQUARE:
	case OSC_TYPE_SAWTOOTH:
		osc->phase = -1;
		break;

	case OSC_TYPE_SINE:
	default:
		osc->phase = 0;		
		break;
	}
	
}

scalar_t osc_fetch_sample(osc_t * osc, scalar_t sample_freq) 
{
	const scalar_t PI_2 = 6.283185307;
	scalar_t sample;

	switch (osc->type) {
	
	case OSC_TYPE_SINE:
	default:
		osc->phase += ((osc->freq / sample_freq) * PI_2);
		while (osc->phase > PI_2)
			osc->phase -= PI_2;
		sample = sin(osc->phase);
		break;
	

	case OSC_TYPE_SAWTOOTH:
		osc->phase += (osc->freq / sample_freq) * 2;
		if (osc->phase > 1)
			osc->phase -= 2;
		sample = osc->phase;
		break;

	case OSC_TYPE_SQUARE:
		osc->phase += (osc->freq / sample_freq) * 2;
		if (osc->phase > 1)
			osc->phase -= 2;
		sample = (osc->phase > 0) ? 1 : -1;
		break;
	
	}

	return sample;
}


