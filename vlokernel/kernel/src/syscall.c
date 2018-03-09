#include "syscall.h"
#include "cpu_state.h"
#include "sched.h"
#include "term.h"

struct cpu_state * syscall(struct cpu_state * cpu)
{
	switch(cpu->eax) {
	case SYSCALL_OUTCHAR:			
		{
			char c, cf, cb;
			c = (char)cpu->ebx;
			cf = (char)cpu->ecx;
			cb = (char)(cpu->ecx >> 8);
			term_setcolor(cf, cb);
			term_putc(c);
			term_setcolor(7, 0);
		}
		break;
		
	case SYSCALL_SCHED: 
		cpu = sched_schedule(cpu);
		gdt_update_tss((uint32_t)(cpu + 1));
		break;
	}
	
	return cpu;
}
