#include "testtasks.h"
#include "cpu_state.h"

#include "term.h"
#include "task.h"
#include "sched.h"
#include "util.h"
#include "multiboot.h"
#include "pmm.h"
#include "elf.h"

#define TESTTASKS_COUNT		6

static void (*testtask_entries[TESTTASKS_COUNT])();

static struct task_state * testtask_idle_state;
static struct task_state * testtask_state_mb_module;
static struct task_state * testtask_states[TESTTASKS_COUNT];

static struct multiboot_mbs_info * testtask_mbs_info;



void testtask_A(void) 																
{
	asm (	"		xor %%eax, %%eax\n"
			"		mov $0x41, %%bl\n"
			"		mov $0x09, %%cl\n"
			"loopa: int $0x30\n"
			"       push %%ecx\n"
			"		movl $0x00ffffff, %%ecx\n"
			"yoloa:	loop yoloa\n"
			"       pop %%ecx\n"
			"       jmp loopa"
			:
			:
			: "ecx", "eax", "bl", "cl"
		);
}

void testtask_B(void) 
{
	asm (	"		xor %%eax, %%eax\n"
			"		mov $0x42, %%bl\n"
			"		mov $0x0a, %%cl\n"
			"loopb: int $0x30\n"
			"       push %%ecx\n"
			"		movl $0x00ffffff, %%ecx\n"
			"yolob:	loop yolob\n"
			"       pop %%ecx\n"
			"       jmp loopb"
			:
			:
			: "ecx", "eax", "bl", "cl"
		);
}

void testtask_C(void) 
{
	asm (	"		xor %%eax, %%eax\n"
			"		mov $0x43, %%bl\n"
			"		mov $0x0b, %%cl\n"
			"loopc: int $0x30\n"
			"       push %%ecx\n"
			"		movl $0x00ffffff, %%ecx\n"
			"yoloc:	loop yoloc\n"
			"       pop %%ecx\n"
			"       jmp loopc"
			:
			:
			: "ecx", "eax", "bl", "cl"
		);
}

void testtask_D(void) 
{
	asm (	"		xor %%eax, %%eax\n"
			"		mov $0x44, %%bl\n"
			"		mov $0x0c, %%cl\n"
			"loopd: int $0x30\n"
			"       push %%ecx\n"
			"		movl $0x00ffffff, %%ecx\n"
			"yolod:	loop yolod\n"
			"       pop %%ecx\n"
			"       jmp loopd"
			:
			:
			: "ecx", "eax", "bl", "cl"
		);
}

void testtask_gpf(void) 
{
	for(;;)
		asm("cli;");
}

void testtask_illegalins(void) 
{
	void (*fuckup)() = (void (*))"\xfe\x20\x20\x20\xde\xad\xbe\xef";
	for (;;)
		fuckup();
}


void testtask_idle(void) 
{
	// syscall #1: yield / reschedule
	asm (	"		xor %%eax, %%eax\n"
			"		inc %%eax\n"
			"idle:	int $0x30\n"
			"		jmp idle"
			:
			:
			: "eax"
	);		
}

struct cpu_state * testtasks_toggle(int task, struct cpu_state * cpu) 
{
	char buf[32];
	
	if (testtask_states[task]) {
		cpu = sched_remove_task(cpu, testtask_states[task]->cpu);
		task_free_user(testtask_states[task]);
		testtask_states[task] = 0;
	} else {
		testtask_states[task] = testtasks_run_module_elf(task);
	}
	
	return cpu;
}

struct task_state * testtasks_run_module_elf(int module_index)
{
	struct task_state * state;
	struct elf_header * elfhdr;
	struct elf_program_header * elfphdr;
	char * srcimage;
	int i, j;
	
	char buf[32];
	struct multiboot_module * modules;
	
	if (testtask_mbs_info->mbs_mods_count <= module_index)
		return;
		
	modules = testtask_mbs_info->mbs_mods_addr;
	
	srcimage = modules[module_index].mod_start;
	elfhdr = srcimage;
	
	// TODO: BAAAD!! Do not trust header blindly
	elfphdr = (struct elf_program_header *)(srcimage + elfhdr->ph_offset);
	for (i = 0; i < elfhdr->ph_entry_count; i++) {
		char * dst = elfphdr->virt_addr;
		char * src = srcimage + elfphdr->offset;
		if (elfphdr->type != 1)
			continue;
		
		for (j = 0; j < elfphdr->mem_size; j++)
			dst[j] = 0;
			
		for (j = 0; j < elfphdr->file_size; j++)
			dst[j] = src[j];
		
		pmm_mark_used(dst);
	}

	state = task_init_user(elfhdr->entry);
	sched_add_task(state->cpu);
	return state;
}

void testtasks_init(struct multiboot_mbs_info * mbs_info) 
{
	int i;

	testtask_mbs_info = mbs_info;
	
	testtask_entries[0] = testtask_A;
	testtask_entries[1] = testtask_B;
	testtask_entries[2] = testtask_C;
	testtask_entries[3] = testtask_D;
	testtask_entries[4] = testtask_gpf;
	testtask_entries[5] = testtask_illegalins;

	testtask_idle_state = task_init_user(testtask_idle);

	sched_add_task(testtask_idle_state->cpu);
	
	

	testtask_state_mb_module = 0;
	for(i = 0; i < TESTTASKS_COUNT; i++) {
		testtask_states[i] = 0;
	}
}

