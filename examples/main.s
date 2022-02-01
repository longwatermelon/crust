.section .data
.LC0: .asciz "    \n"
.LC1: .asciz "string"
.LC3: .asciz "bbbbb"
.LC2: .asciz "aaaaa"
.LC4: .asciz "   \n"
.LC5: .asciz "aaaaa"
.LC6: .asciz "bbbbb"
.LC7: .asciz "ddddd"
.LC8: .asciz "bbbbb"
.LC9: .asciz "bbbbb"
.LC10: .asciz "    \n"
.LC11: .asciz " k  \n"
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
# Push function call args
pushl 8(%ebp)
# Function call
call print
subl $4, %esp
movl %ebx, -4(%ebp)
# Push function call args
pushl 12(%ebp)
# Function call
call print
subl $4, %esp
movl %ebx, -8(%ebp)
# Push function call args
pushl $.LC0
# Function call
call print
subl $4, %esp
movl %ebx, -12(%ebp)
# Return
movl 12(%ebp), %ebx
leave
ret
# Function def
.globl test
test:
pushl %ebp
movl %esp, %ebp
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
movl $.LC1, -8(%ebp)
# Push function call args
pushl $0
pushl $.LC3
pushl $.LC2
# Function call
call func
subl $4, %esp
movl %ebx, -12(%ebp)
# Push function call args
pushl $.LC4
# Function call
call print
subl $4, %esp
movl %ebx, -16(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC5, -20(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC6, -24(%ebp)
# Add value to stack
subl $4, %esp
movl $9, -28(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC7, -32(%ebp)
# Push function call args
pushl $0
pushl $.LC8
pushl $.LC5
# Function call
call func
subl $4, %esp
movl %ebx, -36(%ebp)
# Push function call args
pushl $0
pushl $.LC9
pushl $.LC6
# Function call
call func
subl $4, %esp
movl %ebx, -40(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC6, -44(%ebp)
# Push function call args
pushl $.LC6
# Function call
call print
subl $4, %esp
movl %ebx, -48(%ebp)
# Push function call args
pushl $.LC10
# Function call
call print
subl $4, %esp
movl %ebx, -52(%ebp)
# Assignment: Avoid too many memory references
movl -32(%ebp), %ecx
movl %ecx, -24(%ebp)
# Push function call args
pushl $0
pushl $.LC7
pushl $.LC5
# Function call
call func
subl $4, %esp
movl %ebx, -56(%ebp)
# Get function call return value: avoiding too many memory references
movl -56(%ebp), %ecx
# Assignment
movl %ecx, -44(%ebp)
# Push function call args
pushl $.LC7
# Function call
call print
subl $4, %esp
movl %ebx, -60(%ebp)
# Push function call args
pushl $.LC11
# Function call
call print
subl $4, %esp
movl %ebx, -64(%ebp)
# Return
movl $0, %ebx
leave
ret

