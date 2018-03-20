#include "cpu_state.h"
#include "vmm.h"
#include "util.h"

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
	cpu->esp = (uint32_t)user_stack + VMM_PAGE_SIZE;
	cpu->eip = (uint32_t)entry;
	cpu->cs = 0x18 | 0x03;	
	cpu->ss = 0x20 | 0x03;
	
	cpu->eflags = 0x202;
}

void cpu_state_dump(struct cpu_state * cpu) 
{
	uint8_t * p;
	int i;
	
	vk_printf("   eax=0x%08x  ebx=0x%08x  ecx=0x%08x, edx=0x%08x\n", cpu->eax, cpu->ebx, cpu->ecx, cpu->edx);
	vk_printf("   esi=0x%08x  edi=0x%08x\n", cpu->esi, cpu->edi);
	vk_printf("   esp=0x%08x  ebp=0x%08x  eip=0x%08x\n", cpu->esp, cpu->ebp, cpu->eip);
	vk_printf("eflags=0x%08x   cs=0x%04x       ss=0x%04x\n", cpu->eflags, cpu->cs, cpu->ss);
	
	vk_printf("\ncode @ eip: ");

	p = (uint8_t *)cpu->eip;
	for (i=0; i<32; i++)
		vk_printf("%02x", *p++);
}
