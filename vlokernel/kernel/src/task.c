#include "task.h"
#include "cpu_state.h"
#include "pmm.h"

struct task_state * task_init_kernel(void * entry) 
{
	struct task_state * task = pmm_alloc_page();
	uint8_t * kernel_stack = pmm_alloc_page();

	task->kernel_stack = (void *)kernel_stack;
	task->user_stack = (void *)0;

	struct cpu_state new_state;
	cpu_state_init_ring0(&new_state, entry);
	
	// state auf den task stack kopieren
	struct cpu_state * cpu = (void *) (kernel_stack + PMM_PAGE_SIZE -sizeof(struct cpu_state));
	*cpu = new_state;

	task->cpu = cpu;
	return task;
}


struct task_state * task_init_user(void * entry) 
{
	struct task_state * task = pmm_alloc_page();
	uint8_t * kernel_stack = pmm_alloc_page();
	uint8_t * user_stack = pmm_alloc_page();
	
	task->kernel_stack = (void *)kernel_stack;
	task->user_stack = (void *)user_stack;

	struct cpu_state new_state;
	cpu_state_init_ring3(&new_state, user_stack, entry);
	
	// state auf den task stack kopieren
	struct cpu_state * cpu = (void *) (kernel_stack + PMM_PAGE_SIZE - sizeof(struct cpu_state));
	*cpu = new_state;
	
	task->cpu = cpu;
	return task;
}

void task_free_user(struct task_state * task) 
{
	pmm_free_page(task->kernel_stack);
	pmm_free_page(task->user_stack);
	pmm_free_page(task);
}

void task_free_kernel(struct task_state * task) 
{
	pmm_free_page(task->kernel_stack);
	pmm_free_page(task);
}

