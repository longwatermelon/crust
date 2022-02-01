.section .data
.LC0: .asciz "test\n"
.LC1: .asciz "shit\n"
.LC2: .asciz "ok  \n"
.section .text
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
# Add value to stack
subl $4, %esp
movl $1, -4(%ebp)
# Add value to stack
subl $4, %esp
movl $1, -8(%ebp)
# Binop left
# Avoid too many memory references
movl -4(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -12(%ebp)
# Binop right
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -16(%ebp)
# Prepare registers for math
movl -12(%ebp), %eax
movl -16(%ebp), %ecx
cmpl %eax, %ecx
jne .L1
movl $1, %ecx
jmp .L2
.L1:
movl $0, %ecx
.L2:
cmpl $0, %ecx
je .L4
# Push function call args
pushl $.LC0
# Function call
call print
subl $4, %esp
movl %ebx, -20(%ebp)
.L4:# Assignment
movl $2, -8(%ebp)
# Binop left
# Avoid too many memory references
movl -4(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -24(%ebp)
# Binop right
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -28(%ebp)
# Prepare registers for math
movl -24(%ebp), %eax
movl -28(%ebp), %ecx
cmpl %eax, %ecx
jne .L5
movl $1, %ecx
jmp .L6
.L5:
movl $0, %ecx
.L6:
cmpl $0, %ecx
je .L8
# Push function call args
pushl $.LC1
# Function call
call print
subl $4, %esp
movl %ebx, -32(%ebp)
.L8:# Binop left
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -36(%ebp)
# Binop right
# Add value to stack
subl $4, %esp
movl $2, -40(%ebp)
# Prepare registers for math
movl -36(%ebp), %eax
movl -40(%ebp), %ecx
cmpl %eax, %ecx
jne .L9
movl $1, %ecx
jmp .L10
.L9:
movl $0, %ecx
.L10:
cmpl $0, %ecx
je .L12
# Push function call args
pushl $.LC2
# Function call
call print
subl $4, %esp
movl %ebx, -44(%ebp)
.L12:# Return
movl $0, %ebx
leave
ret

