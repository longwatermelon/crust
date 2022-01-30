.section .data
.LC0: .asciz "test\n"
.globl _start
_start:
call main
mov $1, %eax
int $0x80
# Function def
.globl main
main:
pushl %ebp
movl %esp, %ebp
# Push function call args
pushl $.LC0
# Function call
call print
subl $4, %esp
movl %ebx, -4(%ebp)
# Return
movl $0, %ebx
leave
ret

