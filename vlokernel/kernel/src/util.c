#include "util.h"

void itoa(char * buf, uint32_t val, uint32_t base, uint32_t zpad) 
{
	const char chars[] = "0123456789abcdef";

	char tmp[32] = {0};
	uint32_t i = 30;

	for (;val && i; --i, val /= base) {
		tmp[i] = chars[val % base];
	}

	while (30-i < zpad)
		tmp[i--] = '0';

	i++;

	while (tmp[i]) {
		*buf++ = tmp[i++];
	}
	*buf = 0;
	
}
