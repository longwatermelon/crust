.section .data
.LC0: .asciz "kekw\n"
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
# Push function call args
# Param struct member
movl 8(%ebp), %ebx
movl -12(%ebx), %ecx
pushl %ecx
# Function call
call print
subl $4, %esp
movl %ebx, -4(%ebp)
movl $0, %ebx
leave
ret
# Function def
.globl main
main:
pushl %ebp
movl %esp, %ebp
# Add value to stack
subl $4, %esp
movl $1, -4(%ebp)
# Add value to stack
subl $4, %esp
movl $2, -8(%ebp)
# Add value to stack
subl $4, %esp
movl $1, -12(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC0, -16(%ebp)
# Push function call args
pushl -16(%ebp)
# Function call
call print
subl $4, %esp
movl %ebx, -4(%ebp)
# Push function call args
# Push init list ptr into function call args
leal -4(%ebp), %eax
pushl %eax
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Return
movl $0, %ebx
leave
ret

