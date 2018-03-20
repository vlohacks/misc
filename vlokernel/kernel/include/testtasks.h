#ifndef _TESTTASKS_H
#define _TESTTASKS_H

#include "multiboot.h"
#include "task.h"

void testtasks_init(struct multiboot_mbs_info * mbs_info);
struct cpu_state * testtasks_toggle(int task, struct cpu_state * cpu);
struct task_state * testtasks_run_module_elf(uint32_t module_index);

#endif
