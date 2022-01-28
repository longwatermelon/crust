.section .data
.globl _start
_start:
call main
mov $1, %eax
int $0x80
.globl func
func:
pushl %ebp
movl %esp, %ebp
movl $1, %ebx
leave
ret
.globl other
other:
pushl %ebp
movl %esp, %ebp
movl $2, %ebx
leave
ret
.globl main
main:
pushl %ebp
movl %esp, %ebp
# Push function call args
# Function call
call func
subl $4, %esp
movl %ebx, -4(%ebp)
movl -4(%ebp), %ecx
subl $4, %esp
movl %ecx, -8(%ebp)
# Push function call args
# Function call
call other
subl $4, %esp
movl %ebx, -8(%ebp)
movl -8(%ebp), %ecx
subl $4, %esp
movl %ecx, -12(%ebp)
# Move left operand into eax
movl -8(%ebp), %eax
# Move right operand into ecx
movl -12(%ebp), %ecx
addl %eax, %ecx
movl %ecx, %ebx
leave
ret

