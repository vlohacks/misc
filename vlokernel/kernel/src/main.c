#include "term.h"
#include "gdt.h"
#include "pmm.h"
#include "multiboot.h"
#include "vmm.h"
#include "testtasks.h"
#include "interrupt.h"
#include "cpu_state.h"
#include "pci.h"
#include "util.h"


void kernel_main(struct multiboot_mbs_info * mbs_info) 
{
	vk_setup_io(term_putc, term_puts);
	term_init();

	term_setcolor(14, 0);
	vk_printf("vloOS 0.0.1337 Kernel\n");
	
	term_setcolor(15, 0);

	vk_printf("initializing physical memory manager\n");
	pmm_init(mbs_info);
	
	vk_printf("initializing paging...\n");
	vmm_init();
	vk_printf("WOHOOO we survived :-)\n");
	

	vk_printf("initializing GDT\n");
	gdt_init();
	
	pci_enum();

	interrupt_init();
	
	/*
	uintptr_t base, res;
	base = 0x00001000;
	res = pmm_alloc_page_base(base);
	vk_printf("base=%08x, res=%08x\n", base, res);
	res = pmm_alloc_page_base(base);
	vk_printf("base=%08x, res=%08x\n", base, res);
	*/
	
	
	//vmm_map_page(vmm_get_kernel_context(), 0xa0000, 0xa0000, VMM_PT_PRESENT | VMM_PT_RW);
	//vmm_show_mappings(vmm_get_kernel_context());

	testtasks_init(mbs_info);

	vk_printf("keys:\n1 - 4 : Toggle test tasks\n");
	vk_printf("5     : cause exception: GPF (cli in userspace)\n");
	vk_printf("6     : cause exception: illegal intruction\n");
	vk_printf("7     : Show physical memory map\n");
	vk_printf("8     : process list / stats\n");
	vk_printf("enabling interrupts and just WALK AWAY\n");
	
	term_setcolor(7, 0);
	interrupt_enable();
	
	
	for(;;);
}


