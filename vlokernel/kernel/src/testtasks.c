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
		asm (	"		mov $0x41, %%eax\r\n"
				"		int $0x30\r\n"
				"		movl $0x00ffffff, %%ecx\r\n"
				"yoloa:	loop yoloa"
				:
				:
				: "ecx", "eax"
			);	
	}
}

void testtask_B(void) 
{

	for(;;) { 
		asm (	"		mov $0x42, %%eax\r\n"
				"		int $0x30\r\n"
				"		movl $0x00ffffff, %%ecx\r\n"
				"yolob:	loop yolob"
				:
				:
				: "ecx", "eax"
			);
	}
}

void testtask_C(void) 
{
	for(;;) { 
		asm (	"		mov $0x43, %%eax\r\n"
				"		int $0x30\r\n"
				"		movl $0x00ffffff, %%ecx\r\n"
				"yoloc:	loop yoloc"
				:
				:
				: "ecx", "eax"
			);		
	}
}

void testtask_D(void) 
{
	for(;;) { 
		asm (	"		mov $0x44, %%eax\r\n"
				"		int $0x30\r\n"
				"		movl $0x00ffffff, %%ecx\r\n"
				"yolod:	loop yolod"
				:
				:
				: "ecx", "eax"
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
	for(;;);
}

void testtasks_toggle(int task) 
{
	char buf[32];
	itoa(buf, task, 10, 2);
	term_puts("\ntesttask ");
	term_puts(buf);
	
	if (testtask_states[task]) {
		term_puts(" terminating\n");		
		sched_remove_task(testtask_states[task]->cpu);
		task_free_user(testtask_states[task]);
		testtask_states[task] = 0;
	} else {
		term_puts(" starting\n");
		testtask_states[task] = task_init_user(testtask_entries[task]);
		sched_add_task(testtask_states[task]->cpu);
	}
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

