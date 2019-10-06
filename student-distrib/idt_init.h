#ifndef __IDT_INIT_H
#define __IDT_INIT_H

#define SYSTEM_CALL       0x80
#define KEYBOARD          0x21
#define RTC               0x28
#define PIT               0x20              
#define DEFINE_INTERRUPT  32

// to initialize idt
void idt_init();

// undefine interrupt handler
void undefined_interrupt();

#endif
