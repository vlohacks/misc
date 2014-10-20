#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

const int num_orders = 30;
const int offset_orders = 0x3257b;
const int offset_patterns = 0x32603;
const int offset_samples = 0x34a03;
const int offset_sample_infos = 0x32443;

static const uint16_t const periods[] = {
//    1712,1616,1525,1440,1357,1281,1209,1141,1077,1017, 961, 907,
     856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
     428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
     214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
     107, 101,  95,  90,  85,  80,  76,  71,  67,  64,  60,  57
};

static inline uint16_t swap_endian_u16(uint16_t i) {
    return ((i >> 8) | (i << 8));
}

static inline uint32_t swap_endian_u32(uint32_t i) {
    return ((i>>24)&0xff) | ((i<<8)&0xff0000) | ((i>>8)&0xff00) | ((i<<24)&0xff000000);
}

int rip_sample_infos(FILE * sf, FILE * df, uint16_t * sizes) {
	int i;
	char buf[22];
	fseek(df, 0x14, SEEK_SET);
	fseek(sf, offset_sample_infos, SEEK_SET);
	
	uint16_t w1, w2, w3, w4;
	char c1, c2;
	int samples_total_length = 0;

	for (i = 0; i < 31; i++) {
		fread(&w1, sizeof(uint16_t), 1, sf);
		fread(&w2, sizeof(uint16_t), 1, sf); 	// length
		samples_total_length += w2;
		*sizes++ = w2;
		c1 = fgetc(sf);				// vol
		c2 = fgetc(sf);				// finetune
		fread(&w3, sizeof(uint16_t), 1, sf);
		fread(&w4, sizeof(uint16_t), 1, sf);
		memset(buf, 0, 22);
		if (w2)
			sprintf(buf, "smp %02i", i);
		fwrite(buf, 22, 1, df);
		w2 = swap_endian_u16(w2>>1);
		fwrite(&w2, sizeof(uint16_t), 1, df);
		fputc(c2, df);
		fputc(c1, df);
		
		// loop data not neccessary
		fputc(0, df);
		fputc(0, df);
		fputc(0, df);
		fputc(0, df);
	}
	return samples_total_length;
	
}

int rip_orders(FILE * sf,  FILE * df) {
	int i;
	int num_patterns = -1;
	char c;
	fseek(df, 0x3b6, SEEK_SET);
	fseek(sf, offset_orders, SEEK_SET);
	fputc(num_orders, df);
	fputc(0x7f, df);
	for (i=0; i<128; i++) {
		if (i < num_orders) {
			c = fgetc(sf);
			if (c>num_patterns)
				num_patterns=(int)c;
		} else {
			c = 0;
		}
		
		fputc(c, df);
	}
	fputs("M.K.", df);
	return ++num_patterns;
}

void rip_patterns(FILE * sf, FILE * df, int num_patterns) {
	fseek (df, 0x43c, SEEK_SET);
	fseek(sf, offset_patterns, SEEK_SET);
	num_patterns *= (64*4);
	uint32_t dw1, dw2, tmp;
	char c;
	while (num_patterns--) {
		c = fgetc(sf);	// period index
		tmp = 0;
		if (c)
			tmp |= (periods[c-1]) << 16;

		c = fgetc(sf);	// instrument
		tmp |= (c & 0xf0) << 24;
		tmp |= (c & 0x0f) << 12;
		c = fgetc(sf);	// effect num
		tmp |= (c & 0x0f) << 8;
		c = fgetc(sf);	// effect val
		tmp |= c;
		tmp = swap_endian_u32(tmp);
		fwrite(&tmp, sizeof(uint32_t), 1, df);
	}
}

void rip_samples(FILE * sf, FILE * df, uint16_t * smp_sizes) {

	int i, j;
	int o = offset_samples;
	char * b;
	char * p;
	char c;

	fseek(sf, offset_samples, SEEK_SET);

	for (i = 0; i < 31; i++) {
		printf("%i====\n", smp_sizes[i]);
		if (smp_sizes[i] == 0) 
			continue;
		
		b = malloc(smp_sizes[i]);
		fread(b, 1, smp_sizes[i], sf);
		p=b+smp_sizes[i]-1;
		for (j=0; j<smp_sizes[i]; j++) {
			c = (*p--) ^ 128;
			fputc(c, df);
		}
		free(b);
	}

}

int main(int argc, char ** argv) {
	uint16_t smp_sizes[32];
	FILE * sf = fopen(argv[1], "rb");
	FILE * df = fopen(argv[2], "wb");
	rip_sample_infos(sf, df, smp_sizes);
	int num_patterns = rip_orders(sf, df);
	rip_patterns(sf, df, num_patterns);
	rip_samples(sf, df, smp_sizes);
}


