#include "util.h"

static void (*_putc)(const char);
static void (*_puts)(const char*);

void vk_itoa(char * buf, uint32_t val, uint32_t base, uint32_t padsize, char padchar) 
{
	const char chars[] = "0123456789abcdef";

	char tmp[32] = {0};
	uint32_t i = 30;

	for (;val && i; --i, val /= base) {
		tmp[i] = chars[val % base];
	}

	while (30-i < padsize)
		tmp[i--] = padchar;

	i++;

	while (tmp[i]) {
		*buf++ = tmp[i++];
	}
	*buf = 0;
}

int vk_strlen(char * str) {
	int i;
	for(i = 0; *str != '\0'; str++,i++);
	return i;
		
}

// setup low level i/o functions for text output
void vk_setup_io(void (*putc_func)(const char), void (*puts_func)(const char*)) 
{
	_putc = putc_func;
	_puts = puts_func;
}

// rudimentary printf implementation
static void vk_printf_inner(char * format, int ** va)
{
	int i;
	char fc;
	char padchar;
	char buf[32];
	int padlength;
	int padright;
	
	while ((fc = *(format++)) != 0) {
		if (fc == '%') {
			fc = *(format++);
			if (fc == '\0')
				break;

			padchar = ' ';			
			padlength = 0;
			padright = 0;
			
			if (fc == '-') {
				padright = 1;
				fc = *(format++);
				if (fc == '\0')
					break;
			}
			
			if (fc == '0') {
				padchar = '0';
				fc = *(format++);
				if (fc == '\0')
					break;
			}
			
			if (fc >= '1' && fc <= '9') {
				do {
					padlength *= 10;
					padlength += (fc - '0');
					fc = *(format++);
				} while (fc >= '0' && fc <= '9');
			}			
			
			if (fc == '\0')
				break;
				
			switch(fc) {
			case 'x':
				vk_itoa(buf, (uint32_t)*va, 16, padlength, padchar);
				va++;
				_puts(buf);
				break;
				
			case 'd':
				vk_itoa(buf, (uint32_t)*va, 10, padlength, padchar);
				va++;
				_puts(buf);
				break;
				
			case 's':
				if (padright)
					_puts((char*)*va);
				for (i = 0; i < (padlength-vk_strlen((char *)*va)); i++)
					_putc(padchar);
				if (!padright)
					_puts((char *)*va);
				va++;
				break;
				
			case 'c':
				if (padright)
					_putc((char)*va);
				for (i = 0; i < (padlength-1); i++)
					_putc(padchar);
				if (!padright)
					_putc((char)*va);
				va++;
				break;
				
			default:
				_putc(fc);
			}

		} else {
			_putc(fc);
		}
	}
}

void vk_printf(char * format, ...)
{
	int ** va = ((int **)&format) + 1;
	vk_printf_inner(format, va);
}

void * vk_memcpy(void * dest, const void * src, uint32_t num)
{
	register char * cdest = dest;
	register char * csrc = (char *)src;
	
	while(num--)
		*cdest++ = *csrc++;
		
	return dest;
}

void * vk_memset(void * dest, char val, uint32_t num)
{
	register char * cdest = dest;

	while(num--)
		*cdest++ = val;
		
	return dest;
}
