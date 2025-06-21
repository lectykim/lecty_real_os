state=0
counter=4
nr_system_calls = 3

.globl system_call,sys_fork

.p2align 2
bad_sys_call:
    movl $-1,%eax
    iret
.p2align
reshedule:
    pushl $ret_from_sys_call
    jmp schedule

.p2align 2
system_call:
    cmpl $nr_system_calls-1,%eax
    ja bad_sys_call
    push %ds
    push %es
    push %fs
    pushl %edx
    pushl %ecx
    pushl %ebx
    movl $0x10,%edx
    mov %dx,%ds
    mov %dx, %es
    movl $0x17,%edx
    mov %dx,%fs
    call sys_call_table(.%eax,4)
    pushl %eax
    mocl current,%eax
    cmpl $0,state(%eax)
    jne reshedule
    cmpl $0,counter(%eax)
    je reshedule
.p2align 2
sys_fork:
    call find_empty_process
    testl %eax, %eax
    js 1f
    push %gs
    pushl %esi
    pushl %edi
    pushl %ebp
    pushl %eax
    call copy_process
    addl $20, %esp
1:  ret
