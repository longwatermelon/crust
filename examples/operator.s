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
# Push function call args
# Function call
call other
subl $4, %esp
movl %ebx, -8(%ebp)
# Get function call return value: avoiding too many memory references
movl -8(%ebp), %ecx
# Add value to stack
subl $4, %esp
movl %ecx, -12(%ebp)
# Move left operand into eax
movl -8(%ebp), %eax
# Move right operand into ecx
movl -12(%ebp), %ecx
addl %eax, %ecx
# Return
movl %ecx, %ebx
leave
ret

