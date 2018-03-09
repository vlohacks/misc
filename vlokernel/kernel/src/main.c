#include "term.h"
#include "gdt.h"
#include "pmm.h"
#include "testtasks.h"
#include "interrupt.h"
#include "cpu_state.h"
#include "pci.h"


void kernel_main(struct pmm_mbs_info * mb_info) 
{
	
	char buf[32];

	term_init();
	term_setcolor(14, 0);
	term_puts("vloOS 0.0.1337 Kernel\n");
	term_setcolor(15, 0);
	term_puts("initializing physical memory manager\n");
	pmm_init(mb_info);

	term_puts("initializing GDT\n");
	gdt_init();
	term_puts("initializing IDT\n");
	
	pci_enum();
	
	interrupt_init();
	testtasks_init();
	
	term_puts("keys:\n1 - 4 : Toggle test tasks\n");
	term_puts("5     : cause exception: GPF (cli in userspace)\n");
	term_puts("6     : cause exception: illegal intruction\n");
	term_puts("7     : Show physical memory map\n");
	term_puts("8     : process list / stats\n");
	term_puts("enabling interrupts and just WALK AWAY\n");
	
	term_setcolor(7, 0);
	interrupt_enable();
	
	for(;;);
	
}


