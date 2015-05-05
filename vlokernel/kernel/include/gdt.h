#ifndef _GDT_H
#define _GDT_H

#include "types.h"

#define GDT_FLAG_DATASEG	0x02
#define GDT_FLAG_CODESEG	0x0a
#define GDT_FLAG_TSS		0x09
 
#define GDT_FLAG_SEGMENT	0x10
#define GDT_FLAG_RING0   	0x00
#define GDT_FLAG_RING3   	0x60
#define GDT_FLAG_PRESENT 	0x80
 
#define GDT_FLAG_4K_GRAN 	0x800
#define GDT_FLAG_32_BIT  	0x400

#define GDT_NUM_ENTRIES		6
#define TSS_NUM_ENTRIES		32


void gdt_init();
//static void gdt_set_entry(int i, unsigned int base, unsigned int limit, int flags);
//static void gdt_load();
void gdt_update_tss(uint32_t tss_esp0);

#endif

