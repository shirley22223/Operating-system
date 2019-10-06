#include "idt_handler.h"
#include "lib.h"
#include "rtc.h"
#include "system_calls.h"



/* sys_call_handler
 *
 * Description: handler for system call
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: print "SYS CALL"
 */
void sys_call_handler(){
  printf("SYS CALL\n");			//print "SYS CALL"
}

/* pic_handler
 *
 * Description: handler for PIC
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: print "PIC"
 */
void pic_handler(){
  printf("PIC\n");			//print "PIC"
}


/* Divide_error_exception
 *
 * Description: be called when divide errors happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Divide_error_exception(){
  printf("Divide error exception\n");		//print the exception on screen
  cli();
  while(1){}		//stop the program
}

/* Debug_expection
 *
 * Description: be called when debug exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Debug_expection(){
  printf("Debug exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* NMI_interrupt
 *
 * Description: be called when NMI interrupt happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void NMI_interrupt(){
  printf("NMI interrupt\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Breakpoint_exception
 *
 * Description: be called when Breakpoint exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Breakpoint_exception(){
  printf("Breakpoint exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Overflow_exception
 *
 * Description: be called when overflow happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Overflow_exception(){
  printf("Overflow exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* BOUND_range_exceeded_exception
 *
 * Description: be called when bound range exceeded happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void BOUND_range_exceeded_exception(){
  printf("BOUND range exceeded exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Invalid_opcode_exception
 *
 * Description: be called when invalid opcode happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Invalid_opcode_exception(){
  printf("Invalid opcode exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Device_not_available_exception
 *
 * Description: be called when device not available happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Device_not_available_exception(){
  printf("Device not available exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Double_fault_exception
 *
 * Description: be called when double fault happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Double_fault_exception(){
  printf("Double fault exception\n");		//print the exception on screen
  cli();
  while(1){}		//stop the program
}

/* Coprocessor_segment_overrun
 *
 * Description: be called when coprocessor segment overrun happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Coprocessor_segment_overrun(){
  printf("Coprocessor segment overrun\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Invalid_TSS_exception
 *
 * Description: be called when invalid TSS exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Invalid_TSS_exception(){
  printf("Invalid TSS exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Segment_not_present
 *
 * Description: be called when segment not present happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Segment_not_present(){
  printf("Segment not present\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Stack_fault_exception
 *
 * Description: be called when stack fault exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Stack_fault_exception(){
  printf("Stack fault exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* General_protection_exception
 *
 * Description: be called when general protection exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void General_protection_exception(){
  printf("General protection exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Page_fault_exception
 *
 * Description: be called when page fault exception happen and let program close and return to last program
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: return to last program
 */
void Page_fault_exception(){
  uint32_t cr2_addr;
  asm volatile (
      // CR2 contains the 32-bit addr that caused the fault
      "movl %%cr2, %0;"
      :"=r" (cr2_addr)
    );
  printf("Page fault exception at 0x%x \n", cr2_addr);		//print the exception on screen
  pcb_t* cur_pcb = get_pcb();
  // the program dies because of an exception
  cur_pcb->status_excep = 1;
  //halt the program
  halt((uint8_t)HALT_BY_EXCEP);
}

/* Floating_point_error
 *
 * Description: be called when x87 FPU floating point error happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Floating_point_error(){
  printf("x87 FPU floating point error\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Alignment_check_exception
 *
 * Description: be called when alignment check exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Alignment_check_exception(){
  printf("Alignment check exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* Machine_check_exception
 *
 * Description: be called when machine check exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void Machine_check_exception(){
  printf("Machine check exception\n");		//print the exception on screen
  while(1){}		//stop the program
}

/* SIMD_floating_point_exception
 *
 * Description: be called when SIMD floating point exception happen and let program stop
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: stop the program
 */
void SIMD_floating_point_exception(){
  printf("SIMD floating point exception\n");		//print the exception on screen
  while(1){}		//stop the program
}
