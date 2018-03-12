#ifndef _CPUSTATE_H
#define _CPUSTATE_H

#include "types.h"

struct cpu_state 
{
	// registers we have to take care about in case of interrupt
	uint32_t   eax;
	uint32_t   ebx;
	uint32_t   ecx;
	uint32_t   edx;
	uint32_t   esi;
	uint32_t   edi;
	uint32_t   ebp;
 
	uint32_t   intr;
	uint32_t   error;
 
	// registers saved / restored by the cpu
	uint32_t   eip;
	uint32_t   cs;
	uint32_t   eflags;
	uint32_t   esp;
	uint32_t   ss;
};

void cpu_state_init_ring0(struct cpu_state * cpu, void * entry);
void cpu_state_init_ring3(struct cpu_state * cpu, void * user_stack, void * entry);
void cpu_state_dump(struct cpu_state * cpu);

#endif

