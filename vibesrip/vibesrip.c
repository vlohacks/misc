#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

struct vibes {
	char * data;
	int size;
};

struct modtype {
	uint32_t signature;
	int num_channels;
};

struct modinfo {
	struct modtype type;
	int offset;
	int num_patterns;
	int samples_total_size;
	unsigned char * sample_data_offset;
	char title[32];
};

struct modhdr_sample {
	char name[22];
	uint16_t size;
	int8_t finetune;
	uint8_t volume;
	uint16_t loop_start;
	uint16_t loop_length;
} __attribute__((packed));

struct modhdr {
	char songname[20];
	struct modhdr_sample samples[31];
	uint8_t num_song_positions;
	uint8_t num_load_patterns;
	uint8_t pattern_table[128];
	uint32_t signature;
} __attribute__((packed));

// we search for the following FUCKASS headers
#define NUM_MOD_TYPES 7
const struct modtype mod_types[] = {
	 { 0x2e4b2e4d, 4 },	// M.K.
	 { 0x214b214d, 4 },	// M!K!
	 { 0x34544c46, 4 },	// FLT4
	 { 0x38544c46, 8 },	// FLT8
	 { 0x4e484334, 4 },	// 4CHN
	 { 0x4e484336, 6 },	// 6CHN
	 { 0x4e484338, 8 }	// 8CHN
};

#define HEADER_XOR_MAGIC 0x47
const uint32_t pattern_magic = 0x50415454; // TTAP

/* sorry - too fucking lazy for dynamic allocation....
 * don't cry - this code is way too overengineered 
 * for it's sole purpose ripping off MODs out of 
 * one fuckass specific single production!!!
 */
#define MAX_NUM_MODS 32
struct modinfo mod_infos[MAX_NUM_MODS];

/* load the whole shit onto the heap, nowadays we have
 * FUCKING Lotsa memory, no lameass "sorry pal, 600k needed"
 */
void load_vibes(char * filename, struct vibes * target) {
	FILE * f = fopen(filename, "rb");

	if (f == NULL) {
		target->data = NULL;
		return;
	}
	
	fseek(f, 0, SEEK_END);
	target->size = ftell(f);
	target->data = (char *)malloc(target->size);
	fseek(f, 0, SEEK_SET);

	fread(target->data, target->size, 1, f);

	fclose(f);
}


/* find the fuck ass mods in the mess by identifying their xor-encoded 
 * signature (M.K. 6CHN etc) and the header for rle encoded pattern data
 */
int find_mods(struct vibes * target) {
	int i, j, k;
	int num = 0;
	char hdrdata[4];
	unsigned char * buffer = target->data;
		
	for (j=0; j<(target->size - 8); j++) {
		for (i=0; i<NUM_MOD_TYPES; i++) {
			memcpy(hdrdata, &(mod_types[i].signature), 4);
			for (k=0; k<4; k++) 
				hdrdata[k] ^= HEADER_XOR_MAGIC;

			if (!memcmp(&(buffer[j]), hdrdata, 4)) {
				if (!memcmp(&(buffer[j]) + 8, &pattern_magic, 4)) {
					mod_infos[num].offset = j - (sizeof(struct modhdr) - 4);
					mod_infos[num].type = mod_types[i];

					memcpy (mod_infos[num].title, &(buffer[mod_infos[num].offset]), 32);
					for (k=0; k<32; k++)
						mod_infos[num].title[k] ^= HEADER_XOR_MAGIC;

					mod_infos[num].title[31] = 0; // safety FUCKING ASS first
					num ++;
					break;
				}
			}
		}
	}
	

	return num;
}

/* header data (title, sample slots, pattern order, signature) is stored 
 * fucking bytewise-xored by 0x47 
 * rip_header extracts some other fuck ass data needed for ripping tha 
 * otha shit (sample sizes, num fucking patterns etc)
 */
void rip_header(struct vibes * target, struct modinfo * info, FILE * dest) {
	int i;
	char c;
	char * buffer = target->data;

	struct modhdr header;
	char * header_ptr = (char *) &header;

	for (i=info->offset; i<(info->offset + sizeof(struct modhdr)); i++) 
		*(header_ptr++) = buffer[i] ^ HEADER_XOR_MAGIC; 

	fwrite(&header, sizeof(struct modhdr), 1, dest);

	info->samples_total_size = 0;
	info->num_patterns = 0;

	for (i=0; i<31; i++) {
		if (header.samples[i].size > 1) 
			info->samples_total_size += ((int)header.samples[i].size) << 1;
	}

	for (i=0; i<128; i++) {
		if (info->num_patterns < header.pattern_table[i])
			info->num_patterns = header.pattern_table[i];
	}
	(info->num_patterns)++;
	
}

/* fucking pattern data is stored run-length encoded:
 * 0xff means following data is the fuckass count for rle.
 * if shitass count is 0, a "FUCKING real" 0xff gets written
 * else the next byte is the data written count-times
 * each fucking pattern starts with the sequence TTAP 
 * (PATT for PATTern represented as a little endian integer)
 */
void rip_patterns(struct vibes * target, struct modinfo * info, FILE * dest) {
	int i;
	unsigned char * ptr = target->data + info->offset + sizeof(struct modhdr) + 4;
	int pattern_size;
	unsigned char c;
	unsigned char d;

	for (i=0; i < info->num_patterns; i++) {
		if (*(int*)ptr == pattern_magic) {
			pattern_size = info->type.num_channels * 64 * 4;
			ptr += 4;
			while(pattern_size) {
				c = *ptr++;
				if (c == 0xff) {
					c = *ptr++;
					if (c) {
						d = *ptr++;
						pattern_size -= (int)c; 
						while (c--)
							fputc((int)d, dest);
					} else {
						fputc(0xff, dest);
						pattern_size--;
					}
				} else {
					fputc((int)c, dest);
					pattern_size--;
				}

			}
			
		} else {
			printf("fuck!, incorrect magic :-(\n");
			exit(-1);
		}
	}

	// needed fucking later
	info->sample_data_offset = ptr;
}

/* the lameass sample data is crippled by bytewise xor a char
 * rolled on it's values 0-255 
 * also the sampledata is unsigned 8 bit pcm while signed 8 bit 
 * pcm is standard for MOD files
 */
void rip_samples(struct vibes * target, struct modinfo * info, FILE * dest) {
	unsigned char * ptr = info->sample_data_offset;
	int i = info->samples_total_size;
	unsigned char cunt = 0;
	while(i--)
		fputc ((int)((*ptr++) ^ cunt++) - 128, dest);
}

int main(int argc, char ** argv) {

	struct vibes target;
	int num_mods;
	int i;
	FILE * outf;
	char dstfilename[12];

	printf ("\n");
	printf ("---======= [ VIBES MOD RIPPER ] =======---\n");
	printf ("-- written by vlosoft drecksware in '13 --\n");
	printf ("---====================================---\n\n");

	if (argc < 2) {
		printf("usage: %s <VIBES.EXE>, where VIBES.EXE is your Fucking-Azz unpacked copy of VIBES\n", argv[0]);
		return -1;
	}
		

	load_vibes(argv[1], &target);
	if (target.data == NULL) {
		printf("ERROR while loading your VIBES... make sure the file is FUCKING readable...\n");
		return -1;
	}
	
	num_mods = find_mods(&target);

	if (num_mods) {
		if (num_mods == 12)
			printf("YEAH found the %i MODs!\n\n", num_mods);
		else 
			printf("UMMH found %i MODs... However VIBES normally has 12 MODs\n", num_mods);
	} else {
		printf("sorry pal, 600k^H^H^H^Hno MODs found\n");
		printf("hint: you might unpack the EXE before\n");
		printf("STOP whining and get yourself unp/pklite\n");
		printf("unpacking the shit is not my FUCKING business\n");
		return -1;
	}

	for (i=0; i<num_mods; i++) {
		printf("Ripping a tune called '%s' ...\n", mod_infos[i].title);

		sprintf(dstfilename, "vibes_%02i.mod", i);
		outf = fopen(dstfilename, "wb");

		rip_header(&target, &mod_infos[i], outf);

		printf("  * having %i patterns and %i channels\n", mod_infos[i].num_patterns, mod_infos[i].type.num_channels);

		rip_patterns(&target, &mod_infos[i], outf);
		rip_samples(&target, &mod_infos[i], outf);

		printf("  * DONE ripping to file: '%s'\n\n", dstfilename);
		
		fclose(outf);
	}

	printf("ALL DONE!!\n");
	printf("Greetings to Tonedeaf & Gibson\n");
	printf("all the effort for listening to your\n");
	printf("FUCKING cool tunez in high quality\n\n");

	printf ("---====================================---\n\n");

	// being quite FUCKING unneccessary CLEAN !!!!!!!
	free(target.data);
	
	return 0;
}

