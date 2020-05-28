#define ASM 1
#include "x86_desc.h"

.global rtc_wrapper, keyboard_wrapper, sys_wrapper

#   sys_wrapper
#   discription: wrapper for system calls
#   input: eax, ebx, ecx, edx
#   output: none
#   side effect: none
sys_wrapper:

    cli 
    cmpl $10, %eax
    ja fail
    cmpl $1, %eax 
    jl fail
    
    addl $-8, %esp

    pushw %fs   # save flag
    pushw %es
    pushw %ds

    
    pushl %ebp  # save register
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    
    sti 

    call *sys_call_table(,%eax,4)

    cli 

    popl %ebx   # restore register
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    

    popw %ds    # restore flag
    popw %es
    popw %fs

    addl $8, %esp 

    sti 

    iret

fail:
    movl $-1, %eax 

    sti 
    iret 

sys_call_table:
    .long 0, halt, execute, read, write, open, close, getargs, vidmap


#   keyboard_wrapper
#   discription: wrapper for keyboard interrupt
#   input: none
#   output: none
#   side effect: none
keyboard_wrapper:
    pushal 
    pushfl 
    call keyboard_handler
    popfl 
    popal 
    sti
    iret 

#   rtc_wrapper
#   discription: wrapper for rtc interrupt
#   input: none
#   output: none
#   side effect: none
rtc_wrapper:
    pushal 
    pushfl 
    call rtc_handler
    popfl 
    popal 
    sti
    iret





