#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
	char path[50];
	uint32_t offset;
	uint32_t size;
} fileentry_t;

int main(int argc, char ** argv) {

	FILE 		*fi;
	FILE		*fo;
	uint32_t 	num_entries;
	uint32_t	cob_size;
	fileentry_t 	*entries;
	int		i, j;
	char		*p;
	char		*q;
	char		tmp[128];
	char		tmp2[128];

	fi = fopen(argv[1], "rb");
	fseek(fi, 0, SEEK_END);
	cob_size = (uint32_t)ftell(fi);
	fseek(fi, 0, SEEK_SET);
	
	
	fread(&num_entries, sizeof(uint32_t), 1, fi);
	printf("%u records in COB file\n", num_entries);
	entries = malloc(sizeof(fileentry_t) * num_entries);

	for (i=0; i<num_entries; i++) {
		fread(entries[i].path, 50, 1, fi);
	}

	for (i=0; i<num_entries; i++) {
		fread(&entries[i].offset, sizeof(uint32_t), 1, fi);
	}

	for (i=0; i<num_entries; i++) {
		if (i < (num_entries - 1)) 
			entries[i].size = entries[i+1].offset - entries[i].offset;	
		else 
			entries[i].size = cob_size - entries[i].offset;		
	}

	size_t ttt = ftell(fi);
	printf("==%08x==\n", ttt);

	for (i=0; i<num_entries; i++) {
		for (j=0; j<50; j++) {
			if (entries[i].path[j] == '\\')
				entries[i].path[j] = '/';
		}

		strcpy(tmp, entries[i].path);
		strcpy(tmp2, argv[2]);
		strcat(tmp2, "/");
		p = strtok(tmp, "/");

		while (p) {
			q = p;
			p = strtok(0, "/");
			if (p) {
				strcat(tmp2, q);
				mkdir (tmp2, 0770);
				strcat(tmp2, "/");
			}
		}

		strcat (tmp2, q);
		fo = fopen(tmp2, "wb");
		
		printf(" %s offset=%08x size=%08x\n", entries[i].path, entries[i].offset, entries[i].size);
		fseek(fi, entries[i].offset, SEEK_SET);
		
		 
		p = malloc(entries[i].size);
		fread(p, entries[i].size, 1, fi);
		fwrite(p, entries[i].size, 1, fo);
		free(p);		

		fclose(fo);		

	}
	

}
