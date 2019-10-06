#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "idt_init.h"
#include "idt_handler.h"
#include "idt_linkage.h"

/* undefined_interrupt
 *
 * Description: handler user defined interrupts for interrupts
 *              that are larger than 35
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: print "User Defined Interrupts"
 */
void undefined_interrupt(){
  printf("User Defined Interrupts\n");
}

/* idt_init
 *
 * Description: initialize idt, first set all exceptions to not present
 *              then set all defined interrupts to present and change its
 *              offsits and make everything to interrupt exception gate
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: print the name of each exception
 */
void idt_init(){
  int i;

  for (i = 0; i < NUM_VEC; i++) {
    idt[i].present = 0; // set everything to not present
    idt[i].dpl = 0;
    idt[i].reserved0 = 0;
    idt[i].size = 0;  //set to trap first
    idt[i].reserved1 = 1;
    idt[i].reserved2 = 1;
    idt[i].reserved3 = 1;
    idt[i].reserved4 = 0;
    idt[i].seg_selector = KERNEL_CS; // everything should be kernal mode
    if (i >= DEFINE_INTERRUPT) { // user undefine interrupts
      SET_IDT_ENTRY(idt[i], undefined_interrupt);
    }


    if (i < DEFINE_INTERRUPT){ // set 0-32 to be present with interrupt gate
      idt[i].present = 1;
      idt[i].reserved3 = 0;
    }
  }

//system call interrupt
    idt[SYSTEM_CALL].present = 1; // to be present
    idt[SYSTEM_CALL].dpl = 3; //system call
    idt[SYSTEM_CALL].size = 1;
    idt[SYSTEM_CALL].reserved2 = 1;
    idt[SYSTEM_CALL].reserved3 = 0; // trap gate???????????? Interrupt gate!!!!!
    SET_IDT_ENTRY(idt[SYSTEM_CALL], system_linkage); //set offset for system call(linkage)

// keyboard interrupt
    idt[KEYBOARD].present = 1; // to be present
    idt[KEYBOARD].size = 1;// make it to interrupt gate
    idt[KEYBOARD].reserved2 = 1;
    idt[KEYBOARD].reserved3 = 0;
    SET_IDT_ENTRY(idt[KEYBOARD], keyboard_linkage); //set offset for keyboard(linkage)

// rtc interrupt
    idt[RTC].present = 1; // to be present
    idt[RTC].size = 1;// make it to interrupt gate
    idt[RTC].reserved2 = 1;
    idt[RTC].reserved3 = 0;
    SET_IDT_ENTRY(idt[RTC], rtc_linkage);//set offset for rtc (linkage)

// pit interrupt
    idt[PIT].present = 1; // to be present
    idt[PIT].size = 1;// make it to interrupt gate
    idt[PIT].reserved2 = 1;
    idt[PIT].reserved3 = 0;
    SET_IDT_ENTRY(idt[PIT], pit_linkage); //set offset for pic (linkage)


// interrupts for 0 - 19
SET_IDT_ENTRY(idt[0], Divide_error_exception); // for divide by 0 exception
SET_IDT_ENTRY(idt[1], Debug_expection); // for debug exception
SET_IDT_ENTRY(idt[2], NMI_interrupt); // for NMI interrupt
SET_IDT_ENTRY(idt[3], Breakpoint_exception); // for Breakpoint exception
SET_IDT_ENTRY(idt[4], Overflow_exception);  // for Overflow exception
SET_IDT_ENTRY(idt[5], BOUND_range_exceeded_exception); // for BOUND range exceeded exception
SET_IDT_ENTRY(idt[6], Invalid_opcode_exception); // for Invalid opcode exception
SET_IDT_ENTRY(idt[7], Device_not_available_exception); // for Device not available exception
SET_IDT_ENTRY(idt[8], Double_fault_exception); // for Double fault exception
SET_IDT_ENTRY(idt[9], Coprocessor_segment_overrun); // for Coprocessor segment overrun
SET_IDT_ENTRY(idt[10], Invalid_TSS_exception); // for Invalid TSS exception
SET_IDT_ENTRY(idt[11], Segment_not_present); // for Segment not present
SET_IDT_ENTRY(idt[12], Stack_fault_exception); // for Stack fault exception
SET_IDT_ENTRY(idt[13], General_protection_exception); // for General protection exception
SET_IDT_ENTRY(idt[14], Page_fault_exception); // for Page fault exception
// idt[15] reserved by INTEL
SET_IDT_ENTRY(idt[16], Floating_point_error); // for Floating point error
SET_IDT_ENTRY(idt[17], Alignment_check_exception); // for Alignment check exception
SET_IDT_ENTRY(idt[18], Machine_check_exception); // for Machine check exception
SET_IDT_ENTRY(idt[19], SIMD_floating_point_exception); // for SIMD floating point exception

}
