#ifndef _SCHED_H
#define _SCHED_H

#include "types.h"
#include "vmm.h"
#include "cpu_state.h"

#define SCHED_MAX_TASKS		32
	
struct sched_entity 
{
	uint32_t pid;
	struct cpu_state * cpu;
	struct vmm_context * context;
	struct sched_entity * next;
	struct sched_entity * prev;
};

struct sched_entity * sched_create_task(void * entry);

struct cpu_state * sched_add_task(struct cpu_state * cpu, struct vmm_context * context);
struct cpu_state * sched_remove_task(struct cpu_state * current_cpu, struct cpu_state * cpu_to_kill);
struct cpu_state * sched_schedule(struct cpu_state * cpu);

void sched_ps();

struct sched_entity * sched_list_add(struct sched_entity * parent, struct sched_entity * child);
struct sched_entity * sched_list_remove(struct sched_entity * child);

#endif
