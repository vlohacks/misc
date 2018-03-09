#include "pmm.h"
#include "util.h"
#include "term.h"

static uint32_t pmm_bitmap[PMM_BITMAP_SIZE];
static volatile uint32_t pmm_bottom_used;

extern const void kernel_start;
extern const void kernel_end;

static void pmm_mark_used(void * page) 
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

void pmm_init(struct pmm_mbs_info * mbs_info) 
{
	char buf[64];
	struct pmm_mbs_mmap * mmap = mbs_info->mbs_mmap_addr;
	struct pmm_mbs_mmap * mmap_end = (void *)((uintptr_t *)mbs_info->mbs_mmap_addr + mbs_info->mbs_mmap_length);

	uintptr_t addr, addr_end;
	int i;

	pmm_bottom_used = 32;

	for (i=0; i<PMM_BITMAP_SIZE; i++) 
		pmm_bitmap[i] = 0;

	while (mmap < mmap_end) {

		if (mmap->type == 1) {
			addr = (uintptr_t)mmap->base;
			addr_end = (uintptr_t)(addr + mmap->length);

			term_puts("pmm: freeing         : ");
			itoa(buf, addr, 16, 8);
			term_puts(buf);
			term_puts(" - ");
			itoa(buf, addr_end, 16, 8);
			term_puts(buf);
			term_puts("\n");
		
			while (addr < addr_end) {
				pmm_mark_free((void *)addr);
				addr += PMM_PAGE_SIZE;
			}
		}
		mmap++;
	}

	term_puts("pmm: preserving kmem : ");
	itoa(buf, &kernel_start, 16, 8);
	term_puts(buf);
	term_puts(" - ");
	itoa(buf, &kernel_end, 16, 8);
	term_puts(buf);
	term_puts("\n");

	addr = (uintptr_t) &kernel_start;
	while (addr < (uintptr_t) &kernel_end) {
		pmm_mark_used((void *)addr);
		addr += PMM_PAGE_SIZE;
	}

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
	char buf[9];
	uint32_t i;
/*
	if (limit > PMM_BITMAP_SIZE)
		limit = PMM_BITMAP_SIZE;
*/
	term_puts("physmap pages ");
	itoa(buf, start, 16, 8);
	term_puts(buf);
	term_putc('-');
	itoa(buf, start+limit, 16, 8);
	term_puts(buf);
	term_putc('\n');

	for (i = start; i < (start+limit); i++) {
		itoa(buf, pmm_bitmap[i], 16, 8);
		term_puts(buf);
		term_putc((i+1)&3 ? ' ' : '\n');
	}
}

