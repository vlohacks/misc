#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	char garbage1[10];
	uint32_t num_files;
	char garbage2[50];
} __attribute__((packed)) header_t;

typedef struct {
	char filename[12];
	uint16_t whatever2;
	uint32_t size;
	uint32_t offset;
	char garbage[10];
} __attribute__((packed)) filerecord_t;


uint32_t swap_endian_u32(uint32_t i) {
	return ((i>>24)&0xff) | ((i<<8)&0xff0000) | ((i>>8)&0xff00) | ((i<<24)&0xff000000);
}

int main(int argc, char ** argv) {
	header_t	header;
	filerecord_t 	*recs;
	int		i, j;
	char 		tmp[512];
	char 		*tmp2;

	FILE 		*fi;
	FILE		*fo;
	unsigned char	k;
	
	fi = fopen(argv[1], "rb");

	if (!fi) {
		fprintf(stderr, "Error opening file: %s\n", argv[1]);
		return 1;
	}

	fread(&header, sizeof(header_t), 1, fi);

	printf("Container has %i files...\n", header.num_files);
	recs = malloc(header.num_files * sizeof(filerecord_t));

	for (i=0; i<header.num_files; i++) {
		fread (&(recs[i]), sizeof(filerecord_t), 1, fi);
	}

	for (i=0; i<header.num_files; i++) {

		// Calculate XOR key
		for(j=0,k=0; j<12; j++) 
			k ^= recs[i].filename[j];

		printf("%-12s : offset=%08x size=%08x k=%02x\n", recs[i].filename, recs[i].offset, recs[i].size, k);

		strncpy(tmp, argv[2], 400);
		strcat(tmp, "/");
		strcat(tmp, recs[i].filename);

		fo = fopen(tmp, "wb");
		if (!fo) {
			fprintf(stderr, "Error opening file: %s\n", tmp);
			return 1;
		}
		
		fseek(fi, recs[i].offset, SEEK_SET);

		{	// stack frame
			tmp2 = alloca(recs[i].size);
			fread(tmp2, recs[i].size, 1, fi);

			for (j=0; j<recs[i].size; j++) 
				tmp2[j] ^= k;

			fwrite(tmp2, recs[i].size, 1, fo);
		}
		fclose(fo);
	}

	free(recs);

	fclose(fi);

	

}
