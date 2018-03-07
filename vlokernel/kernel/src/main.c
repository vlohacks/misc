#include "term.h"
#include "gdt.h"
#include "pmm.h"
#include "testtasks.h"
#include "interrupt.h"


void kernel_main(struct pmm_mbs_info * mb_info) {

	term_init();
	term_setcolor(14, 0);
	term_puts("vloOS 0.0.1337 Kernel\n");
	term_setcolor(15, 0);
	term_puts("initializing physical memory manager\n");
	pmm_init(mb_info);
	
	//pmm_show_bitmap(5000);
	
	term_puts("initializing GDT\n");
	gdt_init();
	term_puts("initializing IDT\n");
	interrupt_init();
	testtasks_init();
	term_puts("enabling interrupts\n");
	interrupt_enable();
	term_puts("READY for doing nothing useful....\n");
	term_puts("keys:\n1 - 4 : Toggle test tasks\n");
	term_puts("5     : cause exception: GPF (cli in userspace)\n");
	term_puts("6     : cause exception: illegal intruction\n");
	term_puts("7     : Show physical memory map\n");
	term_puts("8     : ps\n");
	term_setcolor(7, 0);
/*	
	for (i = 0; i < 128; i++) {
		p = pmm_alloc_page();
		itoa(buf, (uint32_t)p, 16, 8);
		term_puts(buf);
		term_puts("\n");
	}


	pmm_show_bitmap(8);
*/
	for(;;);
	
}


