typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

void itoa(char * buf, uint32_t val, uint32_t base, char padc, uint32_t padnum);

// fibonacci testprogram
void _start() 
{
	uint32_t a, b, c;
	char buf[32];
	a = b = 0;
	c = 1;
	for (;;) {
		a += c;
		c = b;
		b = a;
		itoa(buf, a, 10, ' ', 11);
		buf[11] = '\n';
		asm volatile (
				"		mov %0, %%ebx\n"
				"		xor %%ecx, %%ecx\n"
				"		mov $0xd, %%cl\n"
				"		mov $0x2, %%eax\n"
				"		int $0x30\n"
				"		mov $0x00ffffff, %%ecx\n"
				"l:		loop l\n"
				: 
				: "r" (buf) 
				: "eax", "ebx", "ecx"
			);
	}
}

void itoa(char * buf, uint32_t val, uint32_t base, char padc, uint32_t padnum) 
{
	const char chars[] = "0123456789abcdef";

	char tmp[32] = {0};
	uint32_t i = 30;

	for (;val && i; --i, val /= base) {
		tmp[i] = chars[val % base];
	}

	while (30-i < padnum)
		tmp[i--] = padc;

	i++;

	while (tmp[i]) {
		*buf++ = tmp[i++];
	}
	*buf = 0;
}

