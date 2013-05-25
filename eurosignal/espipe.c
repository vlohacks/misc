#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
typedef float scalar_t;

typedef enum {
	OSC_TYPE_SINE = 0
} osc_type_t;

typedef struct {
	osc_type_t	type;
	scalar_t	period;
	scalar_t	freq;
} osc_t;

#define EURONUMBER_LENGTH 6
#define EURONUMBER_BUFFER_MAX 8

const scalar_t eurosignal_freq_table[] =  {
	// Digits
	 979.8,		// f0
	 903.1,		// f1
	 832.5,		// f2
	 767.4,		// f3
	 707.4,		// f4
	 652.0,		// f5
	 601.0,		// f6
	 554.0,		// f7
	 510.7,		// f8
	 470.8,		// f9

	// Special
	 433.9,		// f10
	 400.0,		// f11
	 368.7,		// f12
	 339.9,		// f13
	 313.3,		// f14

	// channel idle
	1153.1,		// fi
	1062.9		// fr
};

int euronumber[EURONUMBER_BUFFER_MAX][EURONUMBER_LENGTH];
int fd;
int euronumber_index_r = 0;
int euronumber_index_w = 0;
int euronumber_pos_w = 0;


scalar_t osc_fetch_sample(osc_t * osc, scalar_t sample_freq) 
{
	const scalar_t PI_2 = 6.283185307;
	scalar_t sample;

	switch (osc->type) {
	
	case OSC_TYPE_SINE:
	default:
		osc->period += ((osc->freq / sample_freq) * PI_2);
		while (osc->period > PI_2)
			osc->period -= PI_2;
		sample = sin(osc->period);
		break;
	}

	return sample;
}

int generate_random_euronumber(int * target) 
{
	int i;
	for (i=0; i<EURONUMBER_LENGTH; i++)
		(*target++) = rand() % 10;
}

void read_fifo() {
	int i, res;
	char c;
	i = euronumber_index_w - euronumber_index_r;
	if (i<0)
		i += EURONUMBER_BUFFER_MAX;

	if (i < (EURONUMBER_BUFFER_MAX - 1)) {
		
		res = read(fd, &c, 1);
		if (c >= '0' && c <= '9') {
			euronumber[euronumber_index_w][euronumber_pos_w++] = (int)(c - '0');
			//fprintf (stderr, "i=%i, c=%c, windex=%i, wpos=%i\n", i, c, euronumber_index_w, euronumber_pos_w);
		}

		if (euronumber_pos_w == 6) {
			euronumber_pos_w = 0;
			euronumber_index_w = (euronumber_index_w + 1) % EURONUMBER_BUFFER_MAX;
		}
	}
}

void make_noise(osc_t * osc, scalar_t freq, scalar_t duration, scalar_t sample_freq) 
{
	int i;
	scalar_t sample;
	int16_t sample_int;
	osc->freq = freq;
	osc->period = 0.0f;
		
	for (i=0; i< (int)(sample_freq * duration); i++) {
		sample = osc_fetch_sample(osc, sample_freq);

		// === add vintage feeling :-) 
		// add white noise
		sample = (sample + ((scalar_t)rand() / RAND_MAX) * 0.2) / 2;

		// overdrive
		sample *= 1.8f;

		// clip
		if (sample > 1.0f)
			sample = 1.0f;
		if (sample < -1.0f)
			sample = -1.0f;
		// ===============================

		// read fifo....
		read_fifo();
		

		sample_int = (int16_t)(sample * 32767);
		fwrite(&sample_int, sizeof(int16_t), 1, stdout);
		fflush(stdout);
	}

}

int main(void) 
{
	scalar_t sample_freq = 44100;
	scalar_t freq;
	osc_t osc;
	int i, j;
	int already_repeated;
	int res;


	fd = open("/opt/es/europipe", O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "could not open pipe");
	}


	srand(time(NULL));

	// init oscillator state
	osc.type = OSC_TYPE_SINE;
	osc.period = 0.0f;
	osc.freq = 440.0f;

	int num_repeat = 3;

	for (;;) {
		i = euronumber_index_w - euronumber_index_r;

		if (i != 0) {
			// transmit random number
			//generate_random_euronumber(euronumber);

			for (j=0; j<num_repeat; j++) {
				already_repeated = 0;
				for (i=0; i<EURONUMBER_LENGTH; i++) {
					fprintf(stderr, "%i", euronumber[euronumber_index_r][i]);
					fflush(stderr);
					if (i > 0) {
						if (euronumber[euronumber_index_r][i-1] == euronumber[euronumber_index_r][i]) {
							if (already_repeated) {
								freq = eurosignal_freq_table[euronumber[euronumber_index_r][i]];
								already_repeated = 0;
							} else {
								freq = eurosignal_freq_table[16];
								already_repeated = 1;
							}
						} else {
							freq = eurosignal_freq_table[euronumber[euronumber_index_r][i]];
							already_repeated = 0;
						}
					} else {
						freq = eurosignal_freq_table[euronumber[euronumber_index_r][i]];
					}		
					make_noise(&osc, freq, 0.1f, sample_freq);
				}
				make_noise(&osc, eurosignal_freq_table[15], 0.22f, sample_freq);
				fprintf(stderr, "%s\n", j ? " (repeated)" : "");
			}
			
			euronumber_index_r = (euronumber_index_r + 1) % EURONUMBER_BUFFER_MAX;
		} else {
			// idle sounds
			make_noise(&osc, eurosignal_freq_table[16], 0.1f, sample_freq);
			make_noise(&osc, eurosignal_freq_table[15], 0.72f, sample_freq);
		}

	}
}
