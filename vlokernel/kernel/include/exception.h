#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "cpu_state.h"

void exception_fuck_system();
void exception_lmaa(struct cpu_state * cpu);
void panic(char * text);

#endif
