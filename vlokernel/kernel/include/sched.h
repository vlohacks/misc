#ifndef _SCHED_H
#define _SCHED_H

#include "types.h"
	
struct sched_entity 
{
	uint32_t pid;
	struct cpu_state * cpu;
	struct sched_entity * next;
	struct sched_entity * prev;
};

struct sched_entity * sched_create_task(void * entry);
void sched_kill_task(struct sched_entity * entity);

struct cpu_state * sched_add_task(struct cpu_state * cpu);
struct cpu_state * sched_remove_task(struct cpu_state * cpu);
struct cpu_state * sched_schedule(struct cpu_state * cpu);

struct sched_entity * sched_list_add(struct sched_entity * parent, struct sched_entity * child);
struct sched_entity * sched_list_remove(struct sched_entity * child);

#endif
