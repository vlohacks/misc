#ifndef _PMM_H
#define _PMM_H

#include "types.h"
#include "multiboot.h"

#define PMM_BITMAP_SIZE 32768
#define PMM_PAGE_SIZE 	4096

void pmm_init(struct multiboot_mbs_info * mbs_info);
void * pmm_alloc_page();
void * pmm_alloc_page_base(void * base);
void pmm_free_page(void * page);
void pmm_show_bitmap(const uint32_t start, const uint32_t limit);
void pmm_mark_used(void * page);

#endif
