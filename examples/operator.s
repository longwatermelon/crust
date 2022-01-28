.section .data
.globl _start
_start:
call main
mov $1, %eax
int $0x80
.globl main
main:
pushl %ebp
movl %esp, %ebp
movl $0, %edx
movl $200, %eax
movl $20, %ecx
idivl %ecx
movl %eax, %ecx
movl %ecx, %ebx
leave
ret

