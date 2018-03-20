#ifndef _VMM_H
#define _VMM_H

#include "types.h"
#include "pmm.h"

#define VMM_PAGE_SIZE				4096

extern const void kernel_end;

// static contigious mapped memory for kernel management structures
// like pagetables and so on
#define VMM_KFIXEDMEM_VIRT_BASE		0x30000000
#define VMM_KFIXEDMEM_PHYS_BASE		0x00500000  //((uint32_t)(&kernel_end + VMM_PAGE_SIZE)) & ~(PMM_PAGE_SIZE-1);
#define VMM_KFIXEDMEM_NUM_PAGES		256
#define VMM_KFIXEDMEM_SIZE		    (VMM_KFIXEDMEM_NUM_PAGES * VMM_PAGE_SIZE)

#define VMM_USERMEM_START 			0x40000000

#define CR0_FLAG_PG					(1<<31)

#define VMM_PT_PRESENT				(1<<0)
#define VMM_PT_RW					(1<<1)
#define VMM_PT_USER					(1<<2)

#define VMM_ERR_SUCCESS				 0
#define VMM_ERR_ALREADY_MAPPED		-1
#define VMM_ERR_NOT_ALIGNED			-2
#define VMM_ERR_BAD_FLAGS			-3
#define VMM_ERR_DOUBLE_FREE			-4

struct vmm_context {
	uint32_t * page_directory_phys;
	uint32_t * page_directory_virt;
	struct vmm_context * self_phys;
};

void vmm_init(void);
void vmm_switch_context(struct vmm_context * context);

static void vmm_alloc_context_internal(struct vmm_context ** context_virt, struct vmm_context ** context_phys);
struct vmm_context * vmm_alloc_context_user(void);
void vmm_free_context(struct vmm_context * context);

int vmm_map_page(struct vmm_context * context, uintptr_t addr_virt, uintptr_t addr_phys, int flags);
int vmm_unmap_page(struct vmm_context * context, uintptr_t addr_virt);
int vmm_alloc_page(struct vmm_context * context, uintptr_t addr_virt, uintptr_t * addr_phys_p, int flags);
int vmm_free_page(struct vmm_context * context, uintptr_t addr_virt);

static void vmm_kfixedmem_alloc_page_internal(uintptr_t * addr_virt, uintptr_t * addr_phys);
void * vmm_kfixedmem_alloc_page();


#endif
