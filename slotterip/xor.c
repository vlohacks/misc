#include <stdio.h>

int main(int argc, char ** argv) {

	FILE 	*fi;
	FILE	*fo;
	int 	i;
	char	c;

	fi = fopen(argv[1], "rb");
	fo = fopen(argv[2], "wb");
		
	do {
		i = fgetc(fi);
		c = (char)i ^ 0x77;
		fputc((int)c, fo);
	} while (i != EOF);
	
}
