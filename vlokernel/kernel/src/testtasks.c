#include "testtasks.h"
#include "cpu_state.h"

#include "term.h"
#include "task.h"
#include "sched.h"
#include "util.h"
#include "multiboot.h"
#include "pmm.h"
#include "elf.h"
#include "exception.h"

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
	if (testtask_states[task]) {
		cpu = sched_remove_task(cpu, testtask_states[task]->cpu);
		task_free_user(testtask_states[task]);
		testtask_states[task] = 0;
	} else {
		testtask_states[task] = testtasks_run_module_elf(task);
	}
	
	return cpu;
}

struct task_state * testtasks_run_module_elf(uint32_t module_index)
{
	struct task_state * state;
	struct elf_header * elfhdr;
	struct elf_program_header * elfphdr;
	struct vmm_context * last_context;
	
	char * srcimage;
	uint32_t i, j;
	int ret;
	
	uintptr_t dst_physpage, src, dst, dst_aligned;
	
	struct multiboot_module * modules;

	if (testtask_mbs_info->mbs_mods_count <= module_index)
		return 0;
		
	modules = testtask_mbs_info->mbs_mods_addr;
	
	srcimage = (char *)modules[module_index].mod_start;
	elfhdr = (struct elf_header *)srcimage;

	// TODO: BAAAD!! Do not trust header blindly
	
	state = task_init_user((void *)elfhdr->entry);
	
	// switch to newly created context, backup currently running context
	last_context = vmm_get_current_context();
	vmm_switch_context(state->context);
	
	elfphdr = (struct elf_program_header *)(srcimage + elfhdr->ph_offset);
	
	for (i = 0; i < elfhdr->ph_entry_count; i++) {
		
		if (elfphdr->type != 1)
			continue;
			
		dst = elfphdr->virt_addr;
		dst_aligned = ((uintptr_t)dst & ~(VMM_PAGE_SIZE - 1));
		src = (uintptr_t)(srcimage + elfphdr->offset);
			
		//vk_printf("loading ELF phdr(%02d): file_size=%08x mem_size=%08x virt_addr=%08x entry=%08x src_offset=%08x\n", i, elfphdr->file_size, elfphdr->mem_size, dst, elfhdr->entry, src);
		
		for (j = 0; j < elfphdr->mem_size; j += VMM_PAGE_SIZE) {
			// map memory in task context
			if ((ret = vmm_alloc_page(state->context, dst_aligned + j, &dst_physpage, VMM_PT_PRESENT | VMM_PT_USER | VMM_PT_RW)) != VMM_ERR_SUCCESS) {
				// ignore double mapping here, it should not harm
				if (ret != VMM_ERR_ALREADY_MAPPED) {
					vk_printf("error: %08x", ret);
					panic("error allocating mem in user context");
				} else {
					//vk_printf("mapped %08x->%08x", dst_aligned + j, dst_physpage);
				}
			}
		}
		
		vk_memset((void*)dst, 0, elfphdr->mem_size);
		vk_memcpy((void*)dst, (void*)src, elfphdr->file_size);
		
		elfphdr++;
	}

	sched_add_task(state->cpu, state->context);
	vmm_switch_context(last_context);
	
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

	testtask_idle_state = task_init_kernel(testtask_idle);//testtasks_run_module_elf(1);//task_init_user(testtask_idle);

	sched_add_task(testtask_idle_state->cpu, testtask_idle_state->context);
	
	

	testtask_state_mb_module = 0;
	for(i = 0; i < TESTTASKS_COUNT; i++) {
		testtask_states[i] = 0;
	}
}

