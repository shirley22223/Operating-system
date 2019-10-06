#ifndef _IDT_HANDLER_H
#define _IDT_HANDLER_H

#ifndef ASM

#include "types.h"

/* handler system call */
extern void sys_call_handler();

/* handler for PIC */
extern void pic_handler();

/* exception for divide error and stops the program */
extern void Divide_error_exception();

/* exception for debug and stops the program */
extern void Debug_expection();

/* exception for NMI interrupt and stops the program */
extern void NMI_interrupt();

/* exception for breakpoint and stops the program */
extern void Breakpoint_exception();

/* exception for overflow and stops the program */
extern void Overflow_exception();

/* exception for bound range exceeded and stops the program */
extern void BOUND_range_exceeded_exception();

/* exception for invalid opcode and stops the program */
extern void Invalid_opcode_exception();

/* exception for device not available and stops the program */
extern void Device_not_available_exception();

/* exception for double fault and stops the program */
extern void Double_fault_exception();

/* exception for coprocessor segment and stops the program */
extern void Coprocessor_segment_overrun();

/* exception for invalid TSS and stops the program */
extern void Invalid_TSS_exception();

/* exception for segment not present and stops the program */
extern void Segment_not_present();

/* exception for stack fault and stops the program */
extern void Stack_fault_exception();

/* exception for general protection and stops the program */
extern void General_protection_exception();

/* exception for page fault and stops the program */
extern void Page_fault_exception();

/* exception for floating point error and stops the program */
extern void Floating_point_error();

/* exception for alignment check and stops the program */
extern void Alignment_check_exception();

/* exception for machine check and stops the program */
extern void Machine_check_exception();

/* exception for SIMD floating point and stops the program */
extern void SIMD_floating_point_exception();

#endif

#endif
