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
	asm volatile("cli");

	term_setcolor(12, 0);

	vk_printf("\n\nLECK MICH AM ARSCH!\n");
	vk_printf("unhandled exception: 0x%02x (%s)\n", cpu->intr, exception_texts[cpu->intr]);
	vk_printf("error code: 0x%08x\n\n", cpu->error);

	cpu_state_dump(cpu);

	for (;;) {
		asm volatile("cli; hlt");
	}

}

void panic(char * text)
{
	vk_printf("OH NO! %s, we all gonna die!\n", text);
	for (;;) {
		asm volatile("cli; hlt");
	}
}

