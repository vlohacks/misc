#ifndef _PMM_H
#define _PMM_H

#include "types.h"

#define PMM_BITMAP_SIZE 32768
#define PMM_PAGE_SIZE 	4096

struct pmm_mbs_mmap 
{
	uint32_t size;
	uint64_t base;
	uint64_t length;
	uint32_t type;
};


struct pmm_mbs_info 
{
	uint32_t mbs_flags;
	uint32_t mbs_mem_lower;
	uint32_t mbs_mem_upper;
	uint32_t mbs_bootdevice;
	char * mbs_cmdline;
	uint32_t mbs_mods_count;
	void * mbs_mods_addr;
	uint32_t mbs_syms[4];
	uint32_t mbs_mmap_length;
	struct pmm_mbs_mmap * mbs_mmap_addr;
		
	// todo Rest wenn nötig
};

void pmm_init(struct pmm_mbs_info * mbs_info);
void * pmm_alloc_page();
void pmm_free_page(void * page);
void pmm_show_bitmap(const uint32_t start, const uint32_t limit);

#endif
