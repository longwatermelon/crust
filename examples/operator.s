.section .data
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
# Get function call return value: avoiding too many memory references
movl -4(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -8(%ebp)
# Binop left
# Binop left
# Avoid too many memory references
movl -8(%ebp), %eax
# Add value to stack
subl $4, %esp
movl %eax, -8(%ebp)
# Binop right
# Add value to stack
subl $4, %esp
movl $2, -12(%ebp)
# Prepare registers for math
movl -8(%ebp), %eax
movl -12(%ebp), %ecx
addl %eax, %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -16(%ebp)
# Binop right
# Push function call args
# Function call
call other
subl $4, %esp
movl %ebx, -24(%ebp)
# Get function call return value: avoiding too many memory references
movl -24(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -20(%ebp)
# Prepare registers for math
movl -16(%ebp), %eax
movl -20(%ebp), %ecx
imull %eax, %ecx
# Return
movl %ecx, %ebx
leave
ret

