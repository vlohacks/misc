#ifndef _VMM_H
#define _VMM_H

#include "types.h"

#define VMM_PAGE_SIZE	4096

#define CR0_FLAG_PG					(1<<31)

#define VMM_PT_PRESENT				(1<<0)
#define VMM_PT_RW					(1<<1)
#define VMM_PT_USER					(1<<2)

#define VMM_ERR_SUCCESS;
#define VMM_ERR_ALREADY_MAPPED		(-1);
#define VMM_ERR_NOT_ALIGNED			(-2);
#define VMM_ERR_BAD_FLAGS			(-3);

struct vmm_context {
	uint32_t * page_directory;
};

void vmm_init(void);
struct vmm_context * vmm_alloc_context(void);
void vmm_free_context(struct vmm_context * context);
int vmm_map_page(struct vmm_context * context, uintptr_t addr_virt, uintptr_t addr_phys, uint32_t flags);


#endif
