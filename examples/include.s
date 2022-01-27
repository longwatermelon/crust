.section .data
.LC0: .asciz "string"
.globl _start
_start:
call main
mov $1, %eax
int $0x80
.globl main
main:
pushl %ebp
movl %esp, %ebp
pushl $.LC0
call print
subl $4, %esp
movl %ebx, -4(%ebp)
movl $0, %ebx
leave
ret

