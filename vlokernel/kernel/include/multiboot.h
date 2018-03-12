#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

#include "types.h"

struct multiboot_mbs_mmap 
{
	uint32_t size;
	uint64_t base;
	uint64_t length;
	uint32_t type;
};


struct multiboot_mbs_info 
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
		
	// there is more in this struct but not required yet
};

struct multiboot_module 
{
	uintptr_t mod_start;
	uintptr_t mod_end;
	char * string;
	uint32_t reserved;
};

#endif
