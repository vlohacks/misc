#include "cpu_state.h"
#include "pmm.h"
#include "util.h"
#include "term.h"

void cpu_state_init_ring0(struct cpu_state * cpu, void * entry) 
{
	cpu->eax = 0;
	cpu->ebx = 0;
	cpu->ecx = 0;
	cpu->edx = 0;
	cpu->esi = 0;
	cpu->edi = 0;
	cpu->ebp = 0;
	//.esp = ??,		// ESP only changes on priv level change
	cpu->eip = (uint32_t)entry;
	cpu->cs = 0x08;		// ring 0 
	//.ss = ??			// SS only changes on priv level change
	
	cpu->eflags = 0x202;
}

void cpu_state_init_ring3(struct cpu_state * cpu, void * user_stack, void * entry) 
{
	cpu->eax = 0;
	cpu->ebx = 0;
	cpu->ecx = 0;
	cpu->edx = 0;
	cpu->esi = 0;
	cpu->edi = 0;
	cpu->ebp = 0;
	cpu->esp = (uint32_t)user_stack + PMM_PAGE_SIZE;
	cpu->eip = (uint32_t)entry;
	cpu->cs = 0x18 | 0x03;	
	cpu->ss = 0x20 | 0x03;
	
	cpu->eflags = 0x202;
}

void cpu_state_dump(struct cpu_state * cpu) 
{
	char buf[32];
	uint8_t * p;
	int i;
	term_puts("eax=0x");	itoa(buf, cpu->eax, 16, 8);	term_puts(buf);
	term_puts("  ebx=0x");	itoa(buf, cpu->ebx, 16, 8);	term_puts(buf);
	term_puts("  ecx=0x");	itoa(buf, cpu->ecx, 16, 8);	term_puts(buf);
	term_puts("  edx=0x");	itoa(buf, cpu->edx, 16, 8);	term_puts(buf);

	term_puts("\nesi=0x");	itoa(buf, cpu->esi, 16, 8);	term_puts(buf);
	term_puts("  edi=0x");	itoa(buf, cpu->edi, 16, 8);	term_puts(buf);

	term_puts("\nesp=0x");	itoa(buf, cpu->esp, 16, 8);	term_puts(buf);
	term_puts("  ebp=0x");	itoa(buf, cpu->ebp, 16, 8);	term_puts(buf);
	term_puts("  eip=0x");	itoa(buf, cpu->eip, 16, 8);	term_puts(buf);

	term_puts("\neflags=0x");	itoa(buf, cpu->eflags, 16, 8);	term_puts(buf);
	term_puts("  cs=0x");	itoa(buf, cpu->cs, 16, 4);	term_puts(buf);
	term_puts("  ss=0x");	itoa(buf, cpu->ss, 16, 4);	term_puts(buf);
	
	term_puts("\n\ncode @ eip: ");
	p = (uint8_t *)cpu->eip;

	for (i=0; i<32; i++) {
		itoa(buf, *p++, 16, 2);
		term_puts(buf);
		term_putc(' ');
	}
	
}
