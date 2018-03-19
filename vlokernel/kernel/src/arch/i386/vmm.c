#include "vmm.h"
#include "pmm.h"
#include "types.h"
#include "util.h"

static struct vmm_context * vmm_kernel_context_phys;
static struct vmm_context * vmm_kernel_context_virt;
static int vmm_paging_enabled;

extern const void kernel_start;
extern const void kernel_end;

// Enable VMM in CR0 register
void vmm_init(void)
{
	int i, ret;
	uintptr_t vaddr;
	uintptr_t paddr;
	
	vmm_paging_enabled = 0;
	vmm_alloc_context_internal(&vmm_kernel_context_virt, &vmm_kernel_context_phys);

	// map context itself since there was no context present at time of creation :-)
	//vmm_map_page(vmm_kernel_context, vmm_kernel_context, vmm_kernel_context, VMM_PT_PRESENT | VMM_PT_RW);
	//vmm_map_page(vmm_kernel_context, vmm_kernel_context->page_directory, vmm_kernel_context->page_directory, VMM_PT_PRESENT | VMM_PT_RW);
	
	// map kfixedmem
	vaddr = VMM_KFIXEDMEM_VIRT_BASE;
	paddr = VMM_KFIXEDMEM_PHYS_BASE;
	for (i = 0; i < VMM_KFIXEDMEM_NUM_PAGES; i++) {
		vmm_map_page(vmm_kernel_context_phys, vaddr, paddr, VMM_PT_PRESENT | VMM_PT_RW);
		vaddr += VMM_PAGE_SIZE;
		paddr += PMM_PAGE_SIZE;
	}
	
	
	/*
	for (i = 0; i < 0xb8000; i+= VMM_PAGE_SIZE) {
		vmm_map_page(vmm_kernel_context, i, i, VMM_PT_PRESENT | VMM_PT_RW);
	}*/
	
	
	vk_printf("mapping VGA memory\n");
	for (i = 0xb8000; i <= 0xbffff; i += VMM_PAGE_SIZE) {
		ret = vmm_map_page(vmm_kernel_context_phys, i, i, VMM_PT_PRESENT | VMM_PT_RW);
		if (ret != VMM_ERR_SUCCESS) {
			vk_printf("error: %08x\n", ret);
			panic("vmm alloc failed");
		}
	}
	
	vk_printf("mapping kernel memory\n");	
	for (i = &kernel_start; i < &kernel_end; i += VMM_PAGE_SIZE) {
		ret = vmm_map_page(vmm_kernel_context_phys, i, i, VMM_PT_PRESENT | VMM_PT_RW);
		if (ret != VMM_ERR_SUCCESS) {
			vk_printf("error: %08x\n", ret);
			panic("vmm alloc failed");
		}
	}
/*	
	// map kernel pagetables created above
	
	for (i = 0; i < (VMM_PAGE_SIZE / sizeof(uint32_t)); i++) {
		if (vmm_kernel_context->page_directory[i] != 0) {
			vk_printf("**PDIndex    %04d %08x\n", i, vmm_kernel_context->page_directory[i] & ~(VMM_PAGE_SIZE - 1));
			uint32_t * pt = vmm_kernel_context->page_directory[i] & ~(VMM_PAGE_SIZE -1);
			for (int j = 0; j < 1024; j++) {
				uint32_t ptentry = pt[j] &  ~(VMM_PAGE_SIZE - 1);
				vmm_map_page(vmm_kernel_context, ptentry, ptentry, VMM_PT_PRESENT | VMM_PT_RW);
				vk_printf("**   PT=%08x\n", pt);
				if (pt[j] != 0)
					vk_printf("**  PDIndex %04d PT=%08x PTIndex %04d %08x\n", i, j, ptentry);
			}
		}
	}	
*/	

	vmm_switch_context(vmm_kernel_context_phys);
		
	// PLATFORM SPECIFIC: Enable Paging
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= CR0_FLAG_PG;
	asm volatile("mov %0, %%cr0" : : "r" (cr0));
	vmm_paging_enabled = 1;
}

static void * vmm_kfixedmem_phys2virt(void * phys_addr)
{
	return (VMM_KFIXEDMEM_VIRT_BASE + ((uint32_t)phys_addr) - VMM_KFIXEDMEM_PHYS_BASE);
}

static void * vmm_kfixedmem_virt2phys(void * virt_addr)
{
	return (((uint32_t)virt_addr + VMM_KFIXEDMEM_PHYS_BASE) - VMM_KFIXEDMEM_VIRT_BASE);
}

struct vmm_context * vmm_get_kernel_context(void)
{
	if (vmm_paging_enabled)
		return vmm_kernel_context_virt;
	else
		return vmm_kernel_context_phys;
}

void vmm_show_mappings(struct vmm_context * context)
{
	/* .... TODO ....
	int i, j;
	for (i = 0; i < 1024; i++) {
		if (context->page_directory[i] != 0) {
			vk_printf("PDIndex    %04d %08x\n", i, context->page_directory[i] & ~(VMM_PAGE_SIZE - 1));
			uint32_t * pt = context->page_directory[i] & ~(VMM_PAGE_SIZE -1);
			for (j = 0; j < 1024; j++) {
				vk_printf("   PT=%08x\n", pt);
				if (pt[j] != 0)
					vk_printf("  PDIndex %04d PT=%08x PTIndex %04d %08x\n", i, j, pt[j] &  ~(VMM_PAGE_SIZE - 1));
			}
		}
	}*/
}

void vmm_switch_context(struct vmm_context * context)
{
	// PLATFORM SPECFIC: Switch context by loading CR3 with PageDirectory
    asm volatile("mov %0, %%cr3" : : "r" (context->page_directory_phys));
}

// allocate initial context (only physical mapping)
static void vmm_alloc_context_internal(struct vmm_context ** context_virt, struct vmm_context ** context_phys) 
{
	int i;
	
	struct vmm_context * contextv;
	struct vmm_context * contextp;
	struct vmm_context * context;
	uint32_t * page_directory;
	
	vmm_kfixedmem_alloc_page_internal(&contextv, &contextp);
	
	if (vmm_paging_enabled)
		context = contextv;
	else
		context = contextp;
	
	vmm_kfixedmem_alloc_page_internal(&(context->page_directory_virt), &(context->page_directory_phys));

	if (vmm_paging_enabled)
		page_directory = context->page_directory_virt;
	else
		page_directory = context->page_directory_phys;

	for (i = 0; i < (PMM_PAGE_SIZE / sizeof(uint32_t)); i++)
		page_directory[i] = 0;
		
	if (context_virt > 0)
		*context_virt = contextv;

	if (context_phys > 0)
		*context_phys = contextp;
		
}


// allocate a vmm context containing a blank page directory, return
// virtual address of context
struct vmm_context * vmm_alloc_context(void) 
{
	struct vmm_context * context;
	vmm_alloc_context_internal(&context, 0);
	return context;
}

// free a complete vmm context including allocated pagetables, page 
// directory and the context itself
void vmm_free_context(struct vmm_context * context) 
{
	/* .... TODO ....
	int i;
	for (i = 0; i < (PMM_PAGE_SIZE / sizeof(uint32_t)); i++) {
		if(context->page_directory[i] != 0)
			pmm_free_page(context->page_directory_phys[i]);
	}
	pmm_free_page(context->page_directory_phys);
	pmm_free_page(context);
	*/
}

int vmm_unmap_page(struct vmm_context * context, uintptr_t addr_virt) 
{
	int i;
	
	uint32_t pagenum = addr_virt / VMM_PAGE_SIZE;
	uint32_t pd_index = pagenum / 1024;
	uint32_t pt_index = pagenum % 1024;

//	uint32_t * page_table = context->page_directory_virt[pd_index];
//  .... TODO .....
	if ((addr_virt & (VMM_PAGE_SIZE - 1)) != 0)
		return VMM_ERR_NOT_ALIGNED;
	
}

int vmm_map_page(struct vmm_context * context, uintptr_t addr_virt, uintptr_t addr_phys, int flags)
{
	int i;
	
	uint32_t pagenum = addr_virt / VMM_PAGE_SIZE;
	uint32_t pd_index = pagenum / 1024;
	uint32_t pt_index = pagenum % 1024;

	uint32_t * page_directory;

	uint32_t * page_table_phys;
	uint32_t * page_table_virt;
	uint32_t * page_table;
	
	// use physical or virtual address of page directory dependent if paging is enabled or not
	if (vmm_paging_enabled)
		page_directory = context->page_directory_virt;
	else
		page_directory = context->page_directory_phys;
	
	// ensure the physical and virtual pages are both 4k aligned
	if ((addr_phys & (PMM_PAGE_SIZE - 1)) != 0)
		return VMM_ERR_NOT_ALIGNED;

	if ((addr_virt & (VMM_PAGE_SIZE - 1)) != 0)
		return VMM_ERR_NOT_ALIGNED;
		
	if ((flags  & ~(VMM_PAGE_SIZE - 1)) != 0)
		return VMM_ERR_BAD_FLAGS;

	if (page_directory[pd_index] & VMM_PT_PRESENT) {
		//vk_printf("PT_EXISTS virt=%08x phys=%08x pd=%08x pt=%08x ctx=%08x\n", addr_virt, addr_phys, pd_index, pt_index, context);
		// use already existing pagetable
		page_table_phys = page_directory[pd_index] & ~(VMM_PAGE_SIZE - 1);
		page_table_virt = vmm_kfixedmem_phys2virt(page_table_phys);
		
		if (vmm_paging_enabled)
			page_table = page_table_virt;
		else 
			page_table = page_table_phys;
	} else {
		//vk_printf("PT_ISNEW  virt=%08x phys=%08x pd=%08x pt=%08x ctx=%08x\n", addr_virt, addr_phys, pd_index, pt_index, context);
		// allocate new page table if there is no record in page directory
		vmm_kfixedmem_alloc_page_internal(&page_table_virt, &page_table_phys);

		// insert physical address of new page table into the page directory
		page_directory[pd_index] = (uint32_t*)((uint32_t)page_table_phys | flags);
		
		// if paging is enabled, initialize page table mapped in virtual address space, else in physical address space
		if (vmm_paging_enabled) {
			for (i = 0; i < (PMM_PAGE_SIZE / sizeof(uint32_t)); i++)
				page_table_virt[i] = 0;
				
			page_table = page_table_virt;
		} else {
			for (i = 0; i < (PMM_PAGE_SIZE / sizeof(uint32_t)); i++)
				page_table_phys[i] = 0;
				
			page_table = page_table_phys;
		}
	}
	
	//vk_printf("=====pt=%08x==pte=%081=\n", page_table, page_table[pt_index]);
	// check if page is already mapped
	if (page_table[pt_index] > 0) 
		return VMM_ERR_ALREADY_MAPPED;
	
	// flags are the lower (unused due to alignment) part of the address
	page_table[pt_index] = addr_phys | flags;
	
	// invalidate TLB for mapped virtual address
	asm volatile ("invlpg %0"  : : "m" (*(char*)addr_virt));
	
	return VMM_ERR_SUCCESS;
}



// allocate a page in the premapped kernel-memory reserved physical memory
static void vmm_kfixedmem_alloc_page_internal(uintptr_t * addr_virt, uintptr_t * addr_phys)
{
	uintptr_t vpage;
	uintptr_t ppage;
	
	ppage = pmm_alloc_page_base(VMM_KFIXEDMEM_PHYS_BASE);
	vpage = vmm_kfixedmem_phys2virt(ppage);

	vk_printf("kfixedmem allocating: %08x->%08x\n", vpage, ppage);
	
	if (((uint32_t)ppage - VMM_KFIXEDMEM_PHYS_BASE) > VMM_KFIXEDMEM_SIZE)
		panic("out of KFIXEDMEM :-(");
		
	if (addr_phys > 0)
		*addr_phys = ppage;
		
	if (addr_virt > 0)
		*addr_virt = vpage;
}

void * vmm_kfixedmem_alloc_page()
{
		uintptr_t vpage;
		vmm_kfixedmem_alloc_page_internal(&vpage, 0);
		return vpage;
}

// allocate physical memory page and map to given virtual address
int vmm_alloc_page(struct vmm_context * context, uintptr_t addr_virt, int flags)
{
	// allocate phys page beyond kernel-reserved memory
	uintptr_t addr_phys = pmm_alloc_page_base(VMM_KFIXEDMEM_PHYS_BASE + VMM_KFIXEDMEM_SIZE);
	return vmm_map_page(context, addr_virt, addr_phys, flags);
}

