#ifndef __FILTER_MOOG_H
#define __FILTER_MOOG_H

#include "types.h"

typedef struct {
	scalar_t f, p, q; //filter coefficients
	scalar_t b0, b1, b2, b3, b4; //filter buffers (beware denormals!)
} filter_moog_t;

void filter_moog_init(filter_moog_t * f);
void filter_moog_set_params(filter_moog_t * f, scalar_t frequency, scalar_t resonance);
void filter_moog_process(filter_moog_t * f, scalar_t * sample);

#endif

