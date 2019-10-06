#ifndef _IDT_LINKAGE_H
#define _IDT_LINKAGE_H

#ifndef ASM

/* Save all current registers and call keyboard handler */
extern void keyboard_linkage();

/* Save all current registers and call rtc handler */
extern void rtc_linkage();

/* Save all current registers and call system call handler */
extern void system_linkage();

/* Save all current registers and call rtc handler */
extern void pit_linkage();

#endif

#endif
