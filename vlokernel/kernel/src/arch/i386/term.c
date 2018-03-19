#include "term.h"

static const size_t TERM_WIDTH 	= 80;
static const size_t TERM_HEIGHT	= 25;

uint32_t	term_row;
uint32_t	term_col;
uint8_t		term_color;

uint16_t	*term_mem;

void term_out(const char c, uint8_t color, size_t x, size_t y) {
	term_mem[y * TERM_WIDTH + x] = ((uint16_t)color) << 8 | c;
}

void term_putc(const char c) 
{
	if (c > 31) 
		term_out(c, term_color, term_col, term_row);

	term_col++;
	if ((term_col >= TERM_WIDTH) || (c == '\n')) {
		term_row++;
		term_col = 0;

		if (term_row >= TERM_HEIGHT) {
			term_scroll(1);
			term_row--;
		}
	}
	
}


void term_scroll(const uint32_t lines) 
{

	size_t i, j;

	if (!lines)
		return;

	for (i=lines;i < TERM_HEIGHT; i++) {
		for (j = 0; j < TERM_WIDTH; j++) 
			term_mem[(i-lines) * TERM_WIDTH + j] = term_mem[i * TERM_WIDTH + j];
	}

	for (i=(TERM_HEIGHT-lines); i < TERM_HEIGHT; i++) {
		for (j = 0; j < TERM_WIDTH; j++)
			term_mem[i * TERM_WIDTH + j] = 0;
	}

}

void term_puts(const char * s) 
{
	while (*s != 0)
		term_putc(*s++);
}

void term_setcolor(const uint8_t fore_color, const uint8_t back_color) 
{
	term_color = (back_color << 4) | fore_color;
}

void term_init(void) 
{
	uint32_t i;

	term_row = 0;
	term_col = 0;
	term_color = 0;
	term_mem = (uint16_t *) 0xb8000;

	for (i = 0; i < (TERM_WIDTH * TERM_HEIGHT); i++) {
		term_mem[i] = 0;
	}
}

