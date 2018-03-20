#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>

#include "types.h"
#include "osc.h"
#include "filter_moog.h"
#include "note.h"

void make_noise(osc_t * osc, int osc_count, scalar_t duration, scalar_t sample_freq, scalar_t decay, filter_moog_t *f) 
{
	int i, j;
	scalar_t sample;
	int16_t sample_int;
	int duration_sec = (int)(sample_freq * duration);
		
	for (i=0; i < duration_sec; i++) {
		sample = 0;
		for (j=0; j<osc_count; j++) {
			sample += osc_fetch_sample(&osc[j], sample_freq);
		}
		sample /= (scalar_t)osc_count;
		
		if (f != NULL)
			filter_moog_process(f, &sample);

		sample *= (duration_sec - i) * decay;
		sample_int = (int16_t)(sample * 32767);
		fwrite(&sample_int, sizeof(int16_t), 1, stdout);
		fflush(stdout);
	}

}


int main(void) 
{
	scalar_t sample_freq = 44100;
	scalar_t i, j, freq;
	scalar_t b;
	int n;

	osc_t osc[12];
	for (n = 0; n<12;n++)
		osc_init(&osc[n], OSC_TYPE_SAWTOOTH);

	filter_moog_t f;
	filter_moog_init(&f);

	i = 0.06f;
	j = -0.01f;
		
	b = 0;
// init oscillator state

	//return 0;

	filter_moog_set_params(&f, 0.05, 0.0f);

	for (;;) {	
		
		
		if ((b+=.2) >= 1) {
			b = 0;
			j *= -1;
		}
/*
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
	*/
		osc[4].freq = note2freq(A, 2) - 1;
		osc[5].freq = note2freq(A, 2) + 1;
		osc[6].freq = note2freq(A, 1);
		osc[7].freq = note2freq(A, 0);
	
		osc[0].freq = note2freq(C, 2) - 1;
		osc[1].freq = note2freq(C, 2) + 1;
		osc[2].freq = note2freq(C, 1);
		osc[3].freq = note2freq(C, 0);

		osc[8].freq = note2freq(E, 2) - 1;
		osc[9].freq = note2freq(E, 2) + 1;
		osc[10].freq = note2freq(E, 1);
		osc[11].freq = note2freq(E, 0);


		i += j;

		make_noise(osc, 12, 0.6f, sample_freq, 0.00001, &f);

		//osc_init(&osc, OSC_TYPE_SAWTOOTH);
		//make_noise(&osc, 2, 2.0f, sample_freq);
		
		//osc_init(&osc, OSC_TYPE_SQUARE);
		//make_noise(&osc, 2, 2.0f, sample_freq);
	}
}
