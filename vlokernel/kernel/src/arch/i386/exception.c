// cause exception for testing :-)
#include "exception.h"
#include "cpu_state.h"
#include "types.h"
#include "term.h"
#include "util.h"

void exception_fuck_system() {
	asm volatile(
		"xorl %%eax, %%eax\n"
		"divl %%eax, %%eax\n"
	 : : :"eax" );
}

static const char * exception_texts[] = {
	"Divide-by-zero Error",
	"Debug",
	"Non-maskable Interrupt",
	"Breakpoint",
	"Overflow",
	"Bound Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocossor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"(reserved)",
	"x87 Floating Point Exception",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating Point Exception",
	"Virtualization Exception",
	"(reserved)",
	"Security Exception",
	"(reserved)",
	"Triple Fault",
	"FPU Error Interrupt"
};

void exception_lmaa(struct cpu_state * cpu) 
{
	char buf[32];

	asm volatile("cli");

	term_setcolor(12, 0);

	term_puts("\n\nLECK MICH AM ARSCH!\n");
	term_puts("unhandled exception: ");
	itoa(buf, cpu->intr, 16, 2);
	term_puts(buf);

	term_puts(" (");
	term_puts(exception_texts[cpu->intr]);
	term_puts(")");
	
	term_puts("\nerror code: 0x");
	itoa(buf, cpu->error, 16, 8);
	term_puts(buf);
	
	term_puts("\n\n");

	cpu_state_dump(cpu);

	for (;;) {
		asm volatile("cli; hlt");
	}

}

