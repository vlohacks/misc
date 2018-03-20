#include "pmm.h"
#include "util.h"

static uint32_t pmm_bitmap[PMM_BITMAP_SIZE];
static volatile uint32_t pmm_bottom_used;

extern const void kernel_start;
extern const void kernel_end;

void pmm_mark_used(void * page) 
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;
	
	pmm_bitmap[bm_index] &= ~(1 << bm_bitindex);
}

static void pmm_mark_free(void * page) 
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;
	
	pmm_bitmap[bm_index] |= (1 << bm_bitindex);
}

void pmm_init(struct multiboot_mbs_info * mbs_info) 
{
	char buf[64];
	struct multiboot_mbs_mmap * mmap = mbs_info->mbs_mmap_addr;
	struct multiboot_mbs_mmap * mmap_end = (void *)((uintptr_t *)mbs_info->mbs_mmap_addr + mbs_info->mbs_mmap_length);

	uintptr_t addr, addr_end;
	int i;

	pmm_bottom_used = 32;

	for (i=0; i<PMM_BITMAP_SIZE; i++) 
		pmm_bitmap[i] = 0;

	vk_printf("test: pmm bitmap addr: 0x%08x\n", pmm_bitmap);
	vk_printf("pmm: freeing memory ...\n");
	while (mmap < mmap_end) {

		if (mmap->type == 1) {
			addr = (uintptr_t)mmap->base;
			addr_end = (uintptr_t)(addr + mmap->length);

			vk_printf("     freeing                : %08x - %08x\n", addr, addr_end);
			
			while (addr < addr_end) {
				pmm_mark_free((void *)addr);
				addr += PMM_PAGE_SIZE;
			}
		}
		mmap++;
	}
	
	vk_printf("pmm: preserving kernel mem  : %08x - %08x\n", (unsigned int)&kernel_start, (unsigned int)&kernel_end);

	addr = (uintptr_t) &kernel_start;
	while (addr < (uintptr_t) &kernel_end) {
		pmm_mark_used((void *)addr);
		addr += PMM_PAGE_SIZE;
	}
	
	// preserve mutliboot structure and modules
	//term_puts("pmm: preserving multiboot structure : ");
	struct multiboot_module * mb_modules = mbs_info->mbs_mods_addr;
	pmm_mark_used(mbs_info);
	pmm_mark_used(mb_modules);
	vk_printf("pmm: preserving memory for multiboot modules...\n");
	for (i = 0; i < mbs_info->mbs_mods_count; i++) {
		addr = mb_modules[i].mod_start;
		while (addr < mb_modules[i].mod_end) {
			pmm_mark_used((void *)addr);
			addr += PMM_PAGE_SIZE;
		}

		vk_printf("     module %-16s: %08x - %08x\n", mb_modules[i].string, mb_modules[i].mod_start, mb_modules[i].mod_end);
	}
}

int pmm_is_page_free(void * page)
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;

	return (pmm_bitmap[bm_index] & (1 << bm_bitindex));
}


void * pmm_alloc_page() 
{
	uintptr_t i, j;
	uintptr_t page;
	
	for (i=pmm_bottom_used; i<PMM_BITMAP_SIZE; i++) {
		if (pmm_bitmap[i]) {
			for (j=0; j<32; j++) {
				if (pmm_bitmap[i] & (1<<j)) {
					pmm_bitmap[i] &= ~(1<<j);
					page = PMM_PAGE_SIZE * ((i << 5) + j);
					pmm_bottom_used = i;
					return (void *)page;
				}
			}
		}
	}
	return 0;
}

void * pmm_alloc_page_base(void * base) 
{
	uintptr_t i, j;
	uintptr_t page;
	uint32_t bm_index = ((uintptr_t)base) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)base) / PMM_PAGE_SIZE) & 31;
	
	if (base == 0)
		bm_index = pmm_bottom_used;
		
	for (i=bm_index; i<PMM_BITMAP_SIZE; i++) {
		if (pmm_bitmap[i]) {
			for (j=(i==0?bm_bitindex:0); j<32; j++) {
				if (pmm_bitmap[i] & (1<<j)) {
					pmm_bitmap[i] &= ~(1<<j);
					page = PMM_PAGE_SIZE * ((i << 5) + j);
					pmm_bottom_used = i;
					return (void *)page;
				}
			}
		}
	}
	return 0;
}

void pmm_free_page(void * page) 
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;
	
	if (bm_index < pmm_bottom_used) {
		if (bm_index >= 32)
			pmm_bottom_used = bm_index;
	}
	
	pmm_bitmap[bm_index] |= (1 << bm_bitindex);
}

void pmm_show_bitmap(const uint32_t start, const uint32_t limit) 
{
	uint32_t i;

	vk_printf("\nphysmap pages 0x%08x - 0x%08x\n", start, start+limit);

	for (i = start; i < (start+limit); i++)
		vk_printf("%08x%c", pmm_bitmap[i], (i+1)&3 ? ' ' : '\n');
}

