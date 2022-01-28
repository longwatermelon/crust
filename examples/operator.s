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
movl $1, %eax
movl $2, %ebx
addl %eax, %ebx
movl %ebx, %ecx
subl $4, %esp
movl %ecx, -4(%ebp)
movl $1, %eax
movl $99, %ebx
addl %eax, %ebx
movl %ebx, %ecx
movl %ecx, %ebx
leave
ret

