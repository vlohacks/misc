#include "testtasks.h"
#include "cpu_state.h"

#include "term.h"
#include "task.h"
#include "sched.h"
#include "util.h"

#define TESTTASKS_COUNT		6

static void (*testtask_entries[TESTTASKS_COUNT])();

static struct task_state * testtask_idle_state;
static struct task_state * testtask_states[TESTTASKS_COUNT];

void testtask_A(void) 																
{
	for(;;) {
		asm (	"		xor %%eax, %%eax\n"
				"		mov $0x41, %%bl\n"
				"		mov $0x09, %%cl\n"
				"		int $0x30\n"
				"		movl $0x00ffffff, %%ecx\n"
				"yoloa:	loop yoloa"
				:
				:
				: "ecx", "eax", "bl", "cl"
			);	
	}
}

void testtask_B(void) 
{

	for(;;) { 
		asm (	"		xor %%eax, %%eax\n"
				"		mov $0x42, %%bl\n"
				"		mov $0x0a, %%cl\n"
				"		int $0x30\n"
				"		movl $0x00ffffff, %%ecx\n"
				"yolob:	loop yolob"
				:
				:
				: "ecx", "eax", "bl", "cl"
			);	
	}
}

void testtask_C(void) 
{
	for(;;) { 
		asm (	"		xor %%eax, %%eax\n"
				"		mov $0x43, %%bl\n"
				"		mov $0x0b, %%cl\n"
				"		int $0x30\n"
				"		movl $0x00ffffff, %%ecx\n"
				"yoloc:	loop yoloc"
				:
				:
				: "ecx", "eax", "bl", "cl"
			);	
	}
}

void testtask_D(void) 
{
	for(;;) { 
		asm (	"		xor %%eax, %%eax\n"
				"		mov $0x44, %%bl\n"
				"		mov $0x0c, %%cl\n"
				"		int $0x30\n"
				"		movl $0x00ffffff, %%ecx\n"
				"yolod:	loop yolod"
				:
				:
				: "ecx", "eax", "bl", "cl"
			);	
	}
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
		testtask_states[task] = task_init_user(testtask_entries[task]);
		sched_add_task(testtask_states[task]->cpu);
	}
	
	return cpu;
}


void testtasks_init() 
{
	int i;

	testtask_entries[0] = testtask_A;
	testtask_entries[1] = testtask_B;
	testtask_entries[2] = testtask_C;
	testtask_entries[3] = testtask_D;
	testtask_entries[4] = testtask_gpf;
	testtask_entries[5] = testtask_illegalins;
	
	testtask_idle_state = task_init_user(testtask_idle);
	sched_add_task(testtask_idle_state->cpu);
	
	
	for(i = 0; i < TESTTASKS_COUNT; i++) {
		testtask_states[i] = 0;
	}
}

