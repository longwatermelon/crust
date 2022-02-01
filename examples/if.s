.section .data
.LC0: .asciz "kekw\n"
.LC1: .asciz "test\n"
.section .text
.globl _start
_start:
call main
mov $1, %eax
int $0x80
# Function def
.globl func
func:
pushl %ebp
movl %esp, %ebp
cmpl $0, 8(%ebp)
je .L1
# Return
movl $0, %ebx
leave
ret
.L1:# Return
movl $1, %ebx
leave
ret
# Function def
.globl main
main:
pushl %ebp
movl %esp, %ebp
# Add value to stack
subl $4, %esp
movl $0, -4(%ebp)
# Add value to stack
subl $4, %esp
movl $1, -8(%ebp)
cmpl $0, -8(%ebp)
je .L2
# Push function call args
pushl $1
# Function call
call func
subl $4, %esp
movl %ebx, -12(%ebp)
# Get function call return value: avoiding too many memory references
movl -12(%ebp), %ecx
cmpl $0, %ecx
je .L3
# Push function call args
pushl $.LC0
# Function call
call print
subl $4, %esp
movl %ebx, -16(%ebp)
.L3:# Push function call args
pushl $.LC1
# Function call
call print
subl $4, %esp
movl %ebx, -20(%ebp)
.L2:# Return
movl $0, %ebx
leave
ret

