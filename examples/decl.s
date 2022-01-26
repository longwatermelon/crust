.section .data
.LC0: .asciz "string"
.LC1: .asciz "string"
.globl _start
_start:
call main
mov $1, %eax
int $0x80
.globl test
test:
pushl %ebp
movl %esp, %ebp
movl $.LC0, %ebx
leave
ret
.globl main
main:
pushl %ebp
movl %esp, %ebp
call test
subl $4, %esp
movl %ebx, -4(%ebp)
movl -4(%ebp), %ecx
subl $4, %esp
movl %ecx, -8(%ebp)
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl -8(%ebp), %ecx
int $0x80
movl $0, %ebx
leave
ret
.globl ex
ex:
pushl %ebp
movl %esp, %ebp
movl $.LC1, %ebx
leave
ret

