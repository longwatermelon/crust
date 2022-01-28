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
movl $10, %eax
movl $2, %ecx
idivl %ecx
movl %eax, %ecx
movl %ecx, %eax
movl $1, %ecx
addl %eax, %ecx
movl %ecx, %eax
movl $2, %ecx
imull %eax, %ecx
movl %ecx, %ebx
leave
ret

