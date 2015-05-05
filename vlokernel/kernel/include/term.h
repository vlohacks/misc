#ifndef _TERM_H
#define _TERM_H

#include "types.h"

void term_setcolor(const uint8_t fore_color, const uint8_t back_color);
void term_init(void);
void term_scroll(const uint32_t lines);
void term_puts(const char * s);
void term_putc(const char c);
void term_putc_noint(const char c);

#endif
