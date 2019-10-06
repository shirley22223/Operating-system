#define ASM     1

.globl get_ebp

get_ebp:
    movl %ebp, %eax
    ret
