#ifndef _SCHEDULE_H
#define _SCHEDULE_H
#include "types.h"

/* counter for opening first three terminals */
extern uint32_t intr_counter;
/* Initialize pit */
void pit_init();
/* Handles pit interrupts */
extern void pit_schedule();
/* Handles context switch */
void context_switch();

#endif
