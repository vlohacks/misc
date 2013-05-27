#include "types.h"
#include "filter_moog.h"

#include <stdio.h>

// Set coefficients given frequency & resonance [0.0...1.0]

void filter_moog_init(filter_moog_t * f) 
{
	f->p = f->q = f->f = 0.0f;
	f->b0 = f->b1 = f->b2 = f->b3 = f->b4 = 0.0f;
}

void filter_moog_set_params(filter_moog_t * f, scalar_t frequency, scalar_t resonance) 
{
	fprintf(stderr, "filter: cutoff=%3.2f, resonance=%3.2f\n", frequency, resonance);
	fflush(stderr);
	f->q = 1.0f - frequency;
	f->p = frequency + 0.8f * frequency * f->q;
	f->f = f->p + f->p - 1.0f;
	f->q = resonance * (1.0f + 0.5f * f->q * (1.0f - f->q + 5.6f * f->q * f->q));
}
// Filter (in [-1.0...+1.0])

void filter_moog_process(filter_moog_t * f, scalar_t * sample) 
{
	scalar_t in, t1, t2;

	in = *sample;
	in -= f->q * f->b4;                          //feedback

	t1 = f->b1;  f->b1 = (in + f->b0) * f->p - f->b1 * f->f;
	t2 = f->b2;  f->b2 = (f->b1 + t1) * f->p - f->b2 * f->f;
	t1 = f->b3;  f->b3 = (f->b2 + t2) * f->p - f->b3 * f->f;

	f->b4 = (f->b3 + t1) * f->p - f->b4 * f->f;
	f->b4 = f->b4 - f->b4 * f->b4 * f->b4 * 0.166667f;    //clipping
	f->b0 = in;

// Lowpass  output:  b4
// Highpass output:  in - b4;
// Bandpass output:  3.0f * (b3 - b4);

	*sample = f->b4;
}
