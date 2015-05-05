#ifndef _TASK_H
#define _TASK_H

#include "types.h"

struct task_state 
{
	void * kernel_stack;
	void * user_stack;
	struct cpu_state * cpu;
};

struct task_state * task_init_kernel(void * entry);
struct task_state * task_init_user(void * entry);
void task_free_user(struct task_state * task);

#endif
