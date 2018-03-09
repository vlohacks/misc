#include "sched.h"
#include "pmm.h"
#include "term.h"
#include "util.h"
#include "cpu_state.h"

static volatile int sched_num_tasks = 0;
static volatile int sched_enabled = 0;

static struct sched_entity * sched_entity_last = 0;
static struct sched_entity * sched_entity_first = 0;
static struct sched_entity * sched_entity_current = 0;

struct cpu_state * sched_add_task(struct cpu_state * cpu) 
{
	if (sched_num_tasks == SCHED_MAX_TASKS)
		return 0;
		
	sched_enabled = 0;
	
	struct sched_entity * entity = pmm_alloc_page();
	entity->cpu = cpu;
	entity->prev = 0;
	entity->next = 0;
	
	if (sched_num_tasks > 0) {
		sched_list_add(sched_entity_last, entity);
	} else {
		sched_entity_first = entity;
	}
	
	sched_entity_last = entity;
	sched_num_tasks++;
	
	sched_enabled = 1;
	return cpu;
}

struct cpu_state * sched_remove_task(struct cpu_state * cpu, struct cpu_state * cpu_to_kill) 
{
	if (sched_num_tasks == 0)
		return 0;
	
	struct sched_entity * entity = sched_entity_first;
	
	// find task by cpu state O(N)
	for(; entity; entity = entity->next) {
		if (entity->cpu == cpu_to_kill)
			break;
	}
	
	if(entity) {
		// task we wanna kill currently runs! Just schedule next instead
		sched_enabled = 0;
		if (entity == sched_entity_current) {
			if (entity->next)
				sched_entity_current = entity->next;
			else
				sched_entity_current = sched_entity_first;
				
			cpu = sched_entity_current->cpu;
		}
		
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

void sched_ps()
{
	char buf[32];
	int i = 0;
	struct sched_entity * tmp = sched_entity_first;
	itoa(buf, sched_num_tasks, 10, 1);
	term_puts("\nnum_tasks : ");
	term_puts(buf);
	term_puts("\n");
	
	term_puts("id st   tstruct      cpu      eax      ebx      ecx      edx\n");
	term_puts("            esp      ebp      esi      edi      eip   eflags\n");
	//         00  R  deadbeef deadbeef deadbeef deadbeef deadbeef deadbeef
	//                deadbeef deadbeef deadbeef deadbeef deadbeef deadbeef
	
	while (tmp != 0) {
		itoa(buf, i, 10, 2);
		term_puts(buf);
		
		term_puts("  ");
		term_putc((tmp == sched_entity_current) ? 'R' : 'S');
		term_puts("  ");

		itoa(buf, tmp, 16, 8);
		term_puts(buf);
		term_putc(' ');

		itoa(buf, tmp->cpu, 16, 8);
    	term_puts(buf);
    	term_putc(' ');
    	
		itoa(buf, tmp->cpu->eax, 16, 8);
    	term_puts(buf);
    	term_putc(' ');

		itoa(buf, tmp->cpu->ebx, 16, 8);
    	term_puts(buf);
    	term_putc(' ');

		itoa(buf, tmp->cpu->ecx, 16, 8);
    	term_puts(buf);
    	term_putc(' ');

		itoa(buf, tmp->cpu->edx, 16, 8);
    	term_puts(buf);
    	term_puts("\n       ");
    	
    	
		itoa(buf, tmp->cpu->esp, 16, 8);
    	term_puts(buf);
    	term_putc(' ');

		itoa(buf, tmp->cpu->ebp, 16, 8);
    	term_puts(buf);
    	term_putc(' ');

		itoa(buf, tmp->cpu->esi, 16, 8);
    	term_puts(buf);
    	term_putc(' ');

		itoa(buf, tmp->cpu->edi, 16, 8);
    	term_puts(buf);    	
    	term_putc(' ');
    	
		itoa(buf, tmp->cpu->eip, 16, 8);
    	term_puts(buf);    	
    	term_putc(' ');

		itoa(buf, tmp->cpu->eflags, 16, 8);
    	term_puts(buf);    	
    	
    	
		term_putc('\n');
		
		//cpu_state_dump(tmp->cpu);

		i++;
		tmp = tmp->next;
	}
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
