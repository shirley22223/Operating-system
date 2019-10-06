#define ASM     1

.globl keyboard_linkage
.globl pit_linkage
.globl rtc_linkage
.globl system_linkage



 # keyboard_linkage
 #
 # Description: Save all current registers and call keyboard_handler,
 # 				and pop back all after return
 # Inputs: None
 # Outputs: None
 # Return Value: None
 # Side Effects: call keyboard_handler
 #
keyboard_linkage:
  pushal	# push all registers
  call keyboard_handler		# call handler funciton
  popal		# pop all registers and return
  iret


 # rtc_linkage
 #
 # Description: Save all current registers and call rtc_handler,
 # 				and pop back all after return
 # Inputs: None
 # Outputs: None
 # Return Value: None
 # Side Effects: call rtc_handler
 #
rtc_linkage:
  pushal	# push all registers
  call rtc_handler		# call handler funciton
  popal		# pop all registers and return
  iret


  # pit_linkage
  #
  # Description: Save all current registers and call pit_schedule,
  # 				and pop back all after return
  # Inputs: None
  # Outputs: None
  # Return Value: None
  # Side Effects: call pit_schedule
  #
pit_linkage:
  pushal	# push all registers
  call pit_schedule		# call handler funciton
  popal		# pop all registers and return
  iret


# Jumptable for system calls
syscall_jumptable:
  .long   0x00, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

 # system_linkage
 #
 # Description: Save all current registers and call sys_call_handler
 #  handler, and pop back all after return
 # Inputs: None
 # Outputs: None
 # Return Value: None
 # Side Effects: call sys_call_handler
 #
system_linkage:
  pushl	   %edi     # push all registers
  pushl    %esi
  pushl    %edx     # first three arguments
  pushl    %ecx
  pushl    %ebx

  # check if eax is within proper range
  cmpl     $1, %eax       # eax = system call number
  jl       error
  cmpl     $10, %eax      # maximum number of system calls: 10
  jg       error
  call *syscall_jumptable(,%eax, 4)   # jump to corresponding function

done:
  popl		  %ebx # pop all registers and return
  popl      %ecx
  popl      %edx
  popl      %esi
  popl      %edi
  iret

error:
  movl $-1, %eax      # return -1 if error
  jmp done
