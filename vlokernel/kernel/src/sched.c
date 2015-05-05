#define SCHED_MAX_TASKS		5
#include "sched.h"
#include "pmm.h"

//static int sched_current_task = -1;
static int sched_num_tasks = 0;
static volatile int sched_enabled = 0;
//static struct cpu_state * sched_task_states[SCHED_MAX_TASKS];

static struct sched_entity * sched_entity_last = 0;
static struct sched_entity * sched_entity_first = 0;
static struct sched_entity * sched_entity_current = 0;

/*
static struct sched_entity sched_entities[SCHED_MAX_TASKS];

static struct sched_entity * sched_first_entity = 0;
static struct sched_entity * sched_current_entity = 0;

void sched_init(void) {
	int i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) {
		sched_entities[i].cpu = 0;
		sched_entities[i].next = 0;
	}
}
*/

struct cpu_state * sched_add_task(struct cpu_state * cpu) 
{
	if (sched_num_tasks == SCHED_MAX_TASKS)
		return 0;
	sched_enabled = 0;
	
	struct sched_entity * entity = pmm_alloc_page();
	entity->cpu = cpu;
	entity->prev = 0;
	entity->next = 0;
	
	if (sched_num_tasks) {
		sched_list_add(sched_entity_last, entity);
	} else {
		sched_entity_first = entity;
	}
	
	sched_entity_last = entity;
	sched_num_tasks++;
	
	sched_enabled = 1;
	return cpu;
}

struct cpu_state * sched_remove_task(struct cpu_state * cpu) 
{
	if (sched_num_tasks == 0)
		return 0;
		
	struct sched_entity * entity = sched_entity_first;
	
	// find task by cpu state O(N)
	for(; entity; entity = entity->next) {
		if (entity->cpu == cpu)
			break;
	}
	
	if(entity) {
		// task we wanna kill currently runs! Just schedule next instead
		sched_enabled = 0;
		//if (entity == sched_entity_current)
		//	sched_entity_current = entity->next;
		
		// task is first in queue - use next as first
		if (entity == sched_entity_first)
			sched_entity_first = entity->next;
			
		if (entity == sched_entity_last)
			sched_entity_last = entity->prev;
					
		sched_list_remove(entity);
		pmm_free_page((void *)entity);
		
		sched_num_tasks--;
		sched_enabled = 1;
	}
	
	
		
	return cpu;
}

struct cpu_state * sched_schedule(struct cpu_state * cpu) 
{
	if (sched_num_tasks == 0) 
		return cpu;
		
	if (!sched_enabled)
		return cpu;

	// first schedule - schedule first task
	if (!sched_entity_current) {
		sched_entity_current = sched_entity_first;
		return sched_entity_current->cpu;
	}
	
	// save CPU state of current task
	sched_entity_current->cpu = cpu;
		
	if (sched_entity_current->next) {
		sched_entity_current = sched_entity_current->next;
	} else {
		sched_entity_current = sched_entity_first;
	}
	
	cpu = sched_entity_current->cpu;

	return cpu;
}

struct sched_entity * sched_list_add(struct sched_entity * parent, struct sched_entity * child) 
{
	struct sched_entity * tmp;

	tmp = parent->next;
	child->prev = parent;
	child->next = tmp;
	parent->next = child;
	
	if (tmp)
		tmp->prev = child;

	return child;
}

struct sched_entity * sched_list_remove(struct sched_entity * child) 
{
	if (child->next != 0)
		child->next->prev = child->prev;

	if (child->prev != 0)
		child->prev->next = child->next;

	child->prev = child->next = 0;

	return child;
}
