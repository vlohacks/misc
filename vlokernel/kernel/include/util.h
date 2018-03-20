#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"

void vk_setup_io(void (*putc_func)(const char), void (*puts_func)(const char*));
void vk_itoa(char * buf, uint32_t val, uint32_t base, uint32_t padsize, char padchar);
void vk_printf(char * format, ...);
void * vk_memcpy(void * dest, const void * src, uint32_t num);
void * vk_memset(void * dest, const char val, uint32_t num);

#endif
