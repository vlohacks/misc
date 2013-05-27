#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>

#include "types.h"
#include "osc.h"
#include "filter_moog.h"

void make_noise(osc_t * osc, int osc_count, scalar_t duration, scalar_t sample_freq, scalar_t decay, filter_moog_t *f) 
{
	int i, j;
	scalar_t sample;
	int16_t sample_int;
		
	for (i=0; i< (int)(sample_freq * duration); i++) {
		sample = 0;
		for (j=0; j<osc_count; j++) {
			sample += osc_fetch_sample(&osc[j], sample_freq);
		}
		sample /= (scalar_t)osc_count;
		
		if (f != NULL)
			filter_moog_process(f, &sample);

		sample *= ((sample_freq * duration) - i) * decay;
		sample_int = (int16_t)(sample * 32767);
		fwrite(&sample_int, sizeof(int16_t), 1, stdout);
		fflush(stdout);
	}

}

typedef enum {
	A	= 0,
	Cb	= 1,
	C 	= 2,
	Db	= 3,
	D 	= 4,
	Eb	= 5,
	E	= 6,
	F	= 7,
	Gb	= 8,
	G	= 9,
	Ab	= 10

} note_t;

scalar_t note2freq(note_t note, int octave) 
{
	const scalar_t sqr2 = 1.0594630943593;
	
	scalar_t freq = 440 * pow(sqr2, note);


	if (octave < 4)
		freq /=  (scalar_t)(1 << (4 - octave));
	else 
		freq *= (scalar_t)(1 << (octave - 4));

	return freq;
}

int main(void) 
{
	scalar_t sample_freq = 44100;
	scalar_t i, j, freq;
	int b;

	osc_t osc[3];
	osc_init(&osc[0], OSC_TYPE_SAWTOOTH);
	osc_init(&osc[1], OSC_TYPE_SAWTOOTH);
	osc_init(&osc[2], OSC_TYPE_SAWTOOTH);

	filter_moog_t f;
	filter_moog_init(&f);

	i = 0.06f;
	j = -0.01f;
		
	b = 0;
// init oscillator state

	for (b=0; b<7; b++) {
		freq = note2freq(A, b);
		fprintf(stderr, "freq=%3.2f, oct=%i\n", freq, b);
		osc[0].freq = freq;
		make_noise(osc, 1, 0.4f, sample_freq, 0.0001, NULL);
	}

	b=0;

	//return 0;

	for (;;) {	
		
		
		if (b++ == 1) {
			b = 0;
			j *= -1;
		}

		osc[0].freq = 109;
		osc[1].freq = 111;
		osc[2].freq = 55;

		i += j;
		filter_moog_set_params(&f, i, 0.6f);

		make_noise(osc, 3, 0.1f, sample_freq, 0.0002, &f);

		osc[0].freq = 219;
		osc[1].freq = 221;
		osc[2].freq = 55;

		i += j;
		filter_moog_set_params(&f, i, 0.6f);

		make_noise(osc, 3, 0.1f, sample_freq, 0.0002, &f);
		
		osc[0].freq = note2freq(Cb, 2) - 1;
		osc[1].freq = note2freq(Cb, 2) + 1;
		osc[2].freq = note2freq(Cb, 1);


		i += j;
		filter_moog_set_params(&f, i, 0.6f);

		make_noise(osc, 3, 0.2f, sample_freq, 0.0001, &f);

		//osc_init(&osc, OSC_TYPE_SAWTOOTH);
		//make_noise(&osc, 2, 2.0f, sample_freq);
		
		//osc_init(&osc, OSC_TYPE_SQUARE);
		//make_noise(&osc, 2, 2.0f, sample_freq);
	}
}
