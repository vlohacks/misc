#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"

void vk_itoa(char * buf, uint32_t val, uint32_t base, int padsize, char padchar);
void vk_printf(char * format, ...);


#endif
