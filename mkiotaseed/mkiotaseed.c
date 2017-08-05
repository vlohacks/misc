#include <stdio.h>
#include <string.h>

const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ9";

#define SEED_LEN 81

int main(void) {
	char buf[SEED_LEN + 1];
	int i, j;
	
	FILE* rand = fopen ("/dev/random", "rb");
	for(i = 0; i < SEED_LEN; i++) {
		do {
			j = fgetc(rand) & 31;
		} while (j > (strlen(alpha) - 1));
		buf[i] = alpha[j];
		printf("%d/%d\n", i, SEED_LEN);
	}
	buf[SEED_LEN] = 0;
	printf("%s\n", buf);
}


