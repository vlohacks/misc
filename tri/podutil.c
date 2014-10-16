#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
	uint32_t num_entries;
	char desc[80];
} pod_header_t;


typedef struct {
	char name[32];
	uint32_t size;
	uint32_t offset;
} pod_entry_t;


void sep_dir(char * dest, char * path) {
	char * p = strtok(path, "\\");
	strcpy(dest, p);	
}

void extract_entry(pod_entry_t * e, char * destfile, FILE * f) {
	size_t tmp = ftell(f);
	char c;
	int i;
	fseek(f, e->offset, SEEK_SET);
	
	FILE * df = fopen(destfile, "wb");
	for (i=0; i<e->size; i++) {
		fread(&c, 1, 1, f);
		fwrite(&c, 1, 1, df);
	}
	fclose (df);
	fseek(f, tmp, SEEK_SET);
}

int main(int argc, char ** argv) {
	
	FILE * f = fopen(argv[1], "rb");

	int i,j;
	pod_header_t hdr;
	pod_entry_t e;
	size_t tmp_offset;
	char tmp[32];
	char tmp2[128];
	fread(&hdr, sizeof(pod_header_t), 1, f);

	printf("num_entries: %u\ndesc: %s\n", hdr.num_entries, hdr.desc);

	for (i=0; i<hdr.num_entries; i++) {
		fread(&e, sizeof(pod_entry_t), 1, f);
		//printf("%-32s %8u %8u\n", e.name, e.size, e.offset);
		strcpy(tmp2, e.name);
		sep_dir(tmp, tmp2);
		*tmp2 = 0;
		strcat(tmp2, "mkdir -p ");
		strcat(tmp2, tmp);
		system(tmp2);
		strcpy(tmp, e.name);
		for (j=0; j<strlen(tmp); j++) {
			if (tmp[j] == '\\')
				tmp[j] = '/';
		}

		printf("%-32s %-32s %8u %8u\n", e.name, tmp, e.size, e.offset);
		extract_entry(&e, tmp, f);
	}

	fclose(f);

}


