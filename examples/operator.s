.section .data
.section .text
.globl _start
_start:
call main
movl $1, %eax
int $0x80
# Function def
.globl func
func:
pushl %ebp
movl %esp, %ebp
# Return
movl $1, %ebx
leave
ret
# Function def
.globl other
other:
pushl %ebp
movl %esp, %ebp
# Return
movl $2, %ebx
leave
ret
# Function def
.globl main
main:
pushl %ebp
movl %esp, %ebp
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -4(%ebp)
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -4(%ebp)
# Get function call return value: avoiding too many memory references
movl -4(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -8(%ebp)
# Binop left
# Binop left
# Binop left
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Get function call return value: avoiding too many memory references
movl -8(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -12(%ebp)
# Binop right
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -16(%ebp)
# Prepare registers for math
movl -12(%ebp), %eax
movl -16(%ebp), %ecx
addl %eax, %ecx
# Binop left
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Get function call return value: avoiding too many memory references
movl -8(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -12(%ebp)
# Binop right
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -16(%ebp)
# Prepare registers for math
movl -12(%ebp), %eax
movl -16(%ebp), %ecx
addl %eax, %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -20(%ebp)
# Binop right
# Add value to stack
subl $4, %esp
movl $2, -24(%ebp)
# Prepare registers for math
movl -20(%ebp), %eax
movl -24(%ebp), %ecx
addl %eax, %ecx
# Binop left
# Binop left
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Get function call return value: avoiding too many memory references
movl -8(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -12(%ebp)
# Binop right
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -16(%ebp)
# Prepare registers for math
movl -12(%ebp), %eax
movl -16(%ebp), %ecx
addl %eax, %ecx
# Binop left
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -8(%ebp)
# Get function call return value: avoiding too many memory references
movl -8(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -12(%ebp)
# Binop right
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -16(%ebp)
# Prepare registers for math
movl -12(%ebp), %eax
movl -16(%ebp), %ecx
addl %eax, %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -20(%ebp)
# Binop right
# Add value to stack
subl $4, %esp
movl $2, -24(%ebp)
# Prepare registers for math
movl -20(%ebp), %eax
movl -24(%ebp), %ecx
addl %eax, %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -28(%ebp)
# Binop right
# Push function call args
# Function call
call other
subl $4, %esp
movl %ebx, -36(%ebp)
# Push function call args
# Function call
call other
subl $4, %esp
movl %ebx, -36(%ebp)
# Get function call return value: avoiding too many memory references
movl -36(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -32(%ebp)
# Prepare registers for math
movl -28(%ebp), %eax
movl -32(%ebp), %ecx
imull %eax, %ecx
# Return
movl %ecx, %ebx
leave
ret

