#include "vmm.h"
#include "pmm.h"
#include "types.h"
#include "util.h"
#include "exception.h"

static struct vmm_context * vmm_kernel_context_phys;
static struct vmm_context * vmm_kernel_context_virt;
static struct vmm_context * vmm_current_context;
static int vmm_paging_enabled;

extern const void kernel_start;
extern const void kernel_end;

static void vmm_alloc_context_internal(struct vmm_context ** context_virt, struct vmm_context ** context_phys);


static void * vmm_kfixedmem_phys2virt(void * phys_addr)
{
	return (void *)(VMM_KFIXEDMEM_VIRT_BASE + ((uintptr_t)phys_addr) - VMM_KFIXEDMEM_PHYS_BASE);
}

static void * vmm_kfixedmem_virt2phys(void * virt_addr)
{
	return (void *)(((uintptr_t)virt_addr + VMM_KFIXEDMEM_PHYS_BASE) - VMM_KFIXEDMEM_VIRT_BASE);
}


// allocate a page in the premapped kernel-memory reserved physical memory
static void vmm_kfixedmem_alloc_page_internal(uintptr_t * addr_virt, uintptr_t * addr_phys)
{
	uintptr_t vpage;
	uintptr_t ppage;
	
	ppage = (uintptr_t)pmm_alloc_page_base((void *)VMM_KFIXEDMEM_PHYS_BASE);
	vpage = (uintptr_t)vmm_kfixedmem_phys2virt((void *)ppage);

	//vk_printf("kfixedmem allocating: %08x->%08x\n", vpage, ppage);
	
	if (((uint32_t)ppage - VMM_KFIXEDMEM_PHYS_BASE) > VMM_KFIXEDMEM_SIZE)
		panic("out of KFIXEDMEM :-(");
		
	if (addr_phys != 0)
		*addr_phys = ppage;
		
	if (addr_virt != 0)
		*addr_virt = vpage;
		
	//vk_printf("==>A(%08x)\n", vpage);
}

void * vmm_kfixedmem_alloc_page()
{
	uintptr_t vpage;
	vmm_kfixedmem_alloc_page_internal(&vpage, 0);
	return (void *)vpage;
}

void vmm_kfixedmem_free_page(void * page)
{
	//vk_printf("==>F(%08x)\n", page);
	pmm_free_page(vmm_kfixedmem_virt2phys(page));
}


// Enable VMM in CR0 register
void vmm_init(void)
{
	uint32_t i;
	int ret;
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

	vk_printf("identity mapping used physmem\n");
	for (i = 0; i < 0x01000000; i += VMM_PAGE_SIZE) {
		if (pmm_is_page_free((void *)i) == 0) {
			//vk_printf("  mapping page: %08x\n", i);
			ret = vmm_map_page(vmm_kernel_context_phys, i, i, VMM_PT_PRESENT | VMM_PT_RW);
			
			if (ret != VMM_ERR_SUCCESS) {
				vk_printf("error: %08x\n", ret);
				panic("Error mapping kfixedmem");
			}
		}
	}

	vmm_switch_context(vmm_kernel_context_phys);
		
	// PLATFORM SPECIFIC: Enable Paging
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= CR0_FLAG_PG;
	asm volatile("mov %0, %%cr0" : : "r" (cr0));
	vmm_paging_enabled = 1;
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

	int i, j;
	uint32_t * page_directory;
	uint32_t * page_table;
	
	if (vmm_paging_enabled)
		page_directory = context->page_directory_virt;
	else
		page_directory = context->page_directory_phys;
	
	for (i = 0; i < 1024; i++) {
		if (page_directory[i] != 0) {
			vk_printf("PDIndex    %04d %08x\n", i, page_directory[i] & ~(VMM_PAGE_SIZE - 1));
			page_table = (uint32_t *)(page_directory[i] & ~(VMM_PAGE_SIZE -1));
			if (vmm_paging_enabled)
				page_table = vmm_kfixedmem_phys2virt(page_table);
			for (j = 0; j < 1024; j++) {
				if (page_table[j] != 0)
					vk_printf("%08x->%08x\n", i * VMM_PAGE_SIZE * 1024 +  (j * 1024), page_table[j] &  ~(VMM_PAGE_SIZE - 1));
			}
		}
	}
}

void vmm_switch_context(struct vmm_context * context)
{
	vmm_current_context = context;
	// PLATFORM SPECFIC: Switch context by loading CR3 with PageDirectory
    asm volatile("mov %0, %%cr3" : : "r" (context->page_directory_phys));
}

struct vmm_context * vmm_get_current_context(void)
{
	return vmm_current_context;
}

// allocate initial context (only physical mapping)
static void vmm_alloc_context_internal(struct vmm_context ** context_virt, struct vmm_context ** context_phys) 
{
	uint32_t i;
	
	struct vmm_context * contextv;
	struct vmm_context * contextp;
	struct vmm_context * context;
	uint32_t * page_directory;
	
	vmm_kfixedmem_alloc_page_internal((uintptr_t *)&contextv, (uintptr_t *)&contextp);
	
	if (vmm_paging_enabled)
		context = contextv;
	else
		context = contextp;
		
	context->self_phys = contextp;
	
	vmm_kfixedmem_alloc_page_internal((uintptr_t *)&(context->page_directory_virt), (uintptr_t *)&(context->page_directory_phys));

	if (vmm_paging_enabled)
		page_directory = context->page_directory_virt;
	else
		page_directory = context->page_directory_phys;

	for (i = 0; i < (PMM_PAGE_SIZE / sizeof(uint32_t)); i++)
		page_directory[i] = 0;
		
	if (context_virt != 0)
		*context_virt = contextv;

	if (context_phys != 0)
		*context_phys = contextp;
		
}


// allocate a vmm context with a copy of kernel context, return
// virtual address of context
struct vmm_context * vmm_alloc_context_user(void) 
{
	struct vmm_context * context;
	int i;
	
	if (!vmm_paging_enabled)
		panic("tried to allocate user context without paging enabled");
	
	vmm_alloc_context_internal(&context, 0);
	for (i = 0; i < (VMM_USERMEM_START / VMM_PAGE_SIZE / 1024); i++) {
		if (vmm_kernel_context_virt->page_directory_virt[i]) {
			//vk_printf("copy kernel mapping: %08x(%08x)\n", vmm_kernel_context_virt->page_directory_virt[i], i * VMM_PAGE_SIZE * 1024);
			context->page_directory_virt[i] = vmm_kernel_context_virt->page_directory_virt[i];
			/*asm volatile(
				"	mov $0x01000000, %%ecx\n"
				"xx:loop xx" : : : "ecx");*/
		}
	}
	
	return context;
}

// free a complete vmm context including allocated pagetables, page 
// directory and the context itself
void vmm_free_context_user(struct vmm_context * context) 
{
	uint32_t * page_table;

	uint32_t pagenum = VMM_USERMEM_START / VMM_PAGE_SIZE;
	uint32_t pd_index = pagenum / 1024;
	uint32_t pt_index = pagenum % 1024;
	
	if (!vmm_paging_enabled)
		panic("tried to free user context without paging enabled");
	
	uint32_t i, j;
	
	// unmap and free all user space memory pages occupied by this context
	for (i = pd_index; i < (PMM_PAGE_SIZE / sizeof(uint32_t)); i++) {
		if(context->page_directory_virt[i] != 0) {
			page_table = vmm_kfixedmem_phys2virt((void *)(context->page_directory_virt[i] & ~(VMM_PAGE_SIZE - 1)));
			for (j = (i==0?pt_index:0); j < (PMM_PAGE_SIZE / sizeof(uint32_t)); j++) {
				if (page_table[j] & VMM_PT_PRESENT) {
					pmm_free_page((void *)(page_table[j] & ~(VMM_PAGE_SIZE - 1)));
					vmm_unmap_page(context, i * VMM_PAGE_SIZE * 1024 +  (j * 1024));	
				}
			}
		}
	}
	
	vmm_kfixedmem_free_page(context->page_directory_virt);
	vmm_kfixedmem_free_page(context);
}

// map physical page into virtual memory.
// takes care itself if paging is enabled or not
int vmm_map_page(struct vmm_context * context, uintptr_t addr_virt, uintptr_t addr_phys, int flags)
{
	uint32_t i;
	
	uint32_t pagenum = addr_virt / VMM_PAGE_SIZE;
	uint32_t pd_index = pagenum / 1024;
	uint32_t pt_index = pagenum % 1024;

	uint32_t * page_directory;
	uint32_t * page_table_phys;
	uint32_t * page_table_virt;
	uint32_t * page_table;
	
	int page_table_empty;
	
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
		page_table_phys = (uint32_t *)(page_directory[pd_index] & ~(VMM_PAGE_SIZE - 1));
		page_table_virt = (uint32_t *)vmm_kfixedmem_phys2virt(page_table_phys);
		
		if (vmm_paging_enabled)
			page_table = page_table_virt;
		else 
			page_table = page_table_phys;
	} else {
		//vk_printf("PT_ISNEW  virt=%08x phys=%08x pd=%08x pt=%08x ctx=%08x\n", addr_virt, addr_phys, pd_index, pt_index, context);
		// allocate new page table if there is no record in page directory
		vmm_kfixedmem_alloc_page_internal((uintptr_t *)&page_table_virt, (uintptr_t *)&page_table_phys);

		// insert physical address of new page table into the page directory
		page_directory[pd_index] = (uint32_t)page_table_phys | flags;
		
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
	if (addr_phys == 0) {
		// Physical address 0: unmap page
		page_table[pt_index] = 0;
		
		// check if page table has still entries, if not unmap it and remove
		// page directory entry
		page_table_empty = 1;
		for (i = 0; i < (PMM_PAGE_SIZE / sizeof(uint32_t)); i++) {
			if (page_table[i] != 0) {
				page_table_empty = 0;
				break;
			}
		}
		if (page_table_empty) {
			vmm_kfixedmem_free_page(page_table_virt);
			page_directory[pd_index] = 0;
		}
	} else {
		// check if page is already mapped
		if (page_table[pt_index] > 0) 
			return VMM_ERR_ALREADY_MAPPED;
	}
	
	// flags are the lower (unused due to alignment) part of the address
	page_table[pt_index] = addr_phys | flags;
	
	// invalidate TLB for mapped virtual address
	asm volatile ("invlpg %0"  : : "m" (*(char*)addr_virt));
	
	return VMM_ERR_SUCCESS;
}

// removes virtual mapping of page
int vmm_unmap_page(struct vmm_context * context, uintptr_t addr_virt)
{
	return vmm_map_page(context, addr_virt, 0, 0);
}

// allocate physical memory page and map to given virtual address.
// Optional return physical page into addr_phys_p
int vmm_alloc_page(struct vmm_context * context, uintptr_t addr_virt, uintptr_t * addr_phys_p, int flags)
{
	int ret;
	
	// allocate phys page beyond kernel-reserved memory
	uintptr_t addr_phys = (uintptr_t)pmm_alloc_page_base((void *)((VMM_KFIXEDMEM_PHYS_BASE + VMM_KFIXEDMEM_SIZE + VMM_PAGE_SIZE) & ~(VMM_PAGE_SIZE - 1)));
	
	ret = vmm_map_page(context, addr_virt, addr_phys, flags);

	if (ret != VMM_ERR_SUCCESS) {
		// in case of error free physical page again 
		pmm_free_page((void *)addr_phys);
	} else {
		// on success return physical pointer if desired by callee
		if (addr_phys_p != 0)
			*addr_phys_p = addr_phys;
	}
	
	return ret;	
}

int vmm_free_page(struct vmm_context * context, uintptr_t addr_virt)
{
	uint32_t pagenum = addr_virt / VMM_PAGE_SIZE;
	uint32_t pd_index = pagenum / 1024;
	uint32_t pt_index = pagenum % 1024;

	uint32_t * page_directory;
	uint32_t * page_table;
	
	if (vmm_paging_enabled)
		page_directory = context->page_directory_virt;
	else
		page_directory = context->page_directory_phys;
		
	page_table = (uint32_t *)(page_directory[pd_index] & ~(VMM_PAGE_SIZE - 1));
	
	if (page_table == 0)
		return VMM_ERR_DOUBLE_FREE;

	if (vmm_paging_enabled)
		page_table = vmm_kfixedmem_phys2virt(page_table);
		
	if (page_table[pt_index] & VMM_PT_PRESENT) {
		pmm_free_page((void *)(page_table[pt_index] & ~(VMM_PAGE_SIZE - 1)));
		return vmm_unmap_page(context, addr_virt);
	} else {
		return VMM_ERR_DOUBLE_FREE;
	}
}





