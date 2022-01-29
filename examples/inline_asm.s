.section .data
.LC0: .asciz "movl $5, %edx"
.LC1: .asciz "movl $1, %ebx"
.LC2: .asciz "movl $4, %eax"
.LC3: .asciz "movl "
.LC4: .asciz ", %ecx"
.LC5: .asciz "int $0x80"
.LC6: .asciz "test\n"
.globl _start
_start:
call main
mov $1, %eax
int $0x80
# Function def
.globl print
print:
pushl %ebp
movl %esp, %ebp
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl 8(%ebp), %ecx
int $0x80
movl $0, %ebx
leave
ret
# Function def
.globl main
main:
pushl %ebp
movl %esp, %ebp
# Push function call args
pushl $.LC6
# Function call
call print
subl $4, %esp
movl %ebx, -4(%ebp)
# Return
movl $0, %ebx
leave
ret

