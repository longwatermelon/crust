.section .data
.LC0: .asciz "test\n"
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
# Add value to stack
subl $4, %esp
movl $1, -4(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC0, -8(%ebp)
# Return
movl -4(%ebp), %ebx
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
movl %ecx, -12(%ebp)
# Push function call args
pushl -16(%ebp)
# Function call
call print
subl $4, %esp
movl %ebx, -12(%ebp)
# Return
movl $0, %ebx
leave
ret

