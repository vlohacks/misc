#include "vmm.h"
#include "pmm.h"
#include "types.h"

static struct vmm_context * vmm_kernel_context;

// Enable VMM in CR0 register
void vmm_init(void)
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= CR0_FLAG_PG;
	asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

struct vmm_context * vmm_alloc_context(void) 
{
	int i;
	struct vmm_context * context = pmm_alloc_page();
	context->page_directory = pmm_alloc_page();
	for (i = 0; i < PMM_PAGE_SIZE; i++)
		context->page_directory[i] = 0;
		
	return context;
}

void vmm_free_context(struct vmm_context * context) 
{
	pmm_free_page(context->page_directory);
	pmm_free_page(context);
}
	
int vmm_map_page(struct vmm_context * context, uintptr_t addr_virt, uintptr_t addr_phys, uint32_t flags)
{
	int i;
	
	uint32_t pagenum = addr_virt / VMM_PAGE_SIZE;
	uint32_t pd_index = pagenum / 1024;
	uint32_t pt_index = pagenum % 1024;
	uint32_t * page_table = context->page_directory[pd_index];
	
	// ensure the physical and virtual pages are both 4k aligned
	if ((addr_phys & (PMM_PAGE_SIZE - 1)) != 0)
		return VMM_ERR_NOT_ALIGNED;

	if ((addr_virt & (VMM_PAGE_SIZE - 1)) != 0)
		return VMM_ERR_NOT_ALIGNED;
		
	if ((flags  & ~(VMM_PAGE_SIZE - 1)) != 0)
		return VMM_ERR_BAD_FLAGS;
	
	// allocate new page table if there is no record in page directory
	if (page_table == 0) {
		page_table = pmm_alloc_page();
		for (i = 0; i < PMM_PAGE_SIZE; i++)
			page_table[i] = 0;
		context->page_directory[pd_index] = (uint32_t*)((uint32_t)page_table | flags);
	}
	
	// chech if page is already mapped
	if (page_table[pt_index] > 0) 
		return VMM_ERR_ALREADY_MAPPED;
	
	// flags are the lower (unused due to alignment) part of the address
	page_table[pt_index] = addr_phys | flags;
	
	// invalidate TLB for mapped virtual address
	asm volatile ("invlpg %0"  : : "m" (*(char*)addr_virt));
	
	return VMM_ERR_SUCCESS;
}



