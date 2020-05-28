
.text
.globl EXEC_TO_USER

#   exec_to_user
#   description: push the artificial iret on stack
#   input: USER_DS, virtual address, entry point
#   output: none
#   side effect: none
EXEC_TO_USER:
    
    movl 4(%esp), %ebx
    movl 8(%esp), %ecx
    movl 12(%esp), %esi
    movl 16(%esp), %edi
    mov %ebx, %ds
    pushl %ebx
    pushl %ecx
    pushfl
    pushl %esi
    pushl %edi
    iret
IRET_RETURN:
    leave
    ret
.end
