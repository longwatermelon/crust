.section .data
.LC0: .asciz "    \n"
.LC1: .int 1
.LC2: .asciz "string"
.LC5: .int 0
.LC4: .asciz "bbbbb"
.LC3: .asciz "aaaaa"
.LC6: .asciz "   \n"
.LC7: .asciz "aaaaa"
.LC8: .asciz "bbbbb"
.LC9: .int 9
.LC10: .asciz "ddddd"
.LC12: .int 0
.LC11: .asciz "bbbbb"
.LC14: .int 0
.LC13: .asciz "bbbbb"
.LC15: .asciz "    \n"
.LC16: .int 0
.LC17: .asciz " k  \n"
.LC18: .int 0
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
# Avoid too many memory references
movl 1, %eax
# Add value to stack
subl $4, %esp
movl %eax, -4(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC2, -8(%ebp)
# Push function call args
pushl 0
pushl $.LC4
pushl $.LC3
# Function call
call func
subl $4, %esp
movl %ebx, -12(%ebp)
# Push function call args
pushl $.LC6
# Function call
call print
subl $4, %esp
movl %ebx, -16(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC7, -20(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC8, -24(%ebp)
# Avoid too many memory references
movl 9, %eax
# Add value to stack
subl $4, %esp
movl %eax, -28(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC10, -32(%ebp)
# Push function call args
pushl 0
pushl $.LC11
pushl $.LC7
# Function call
call func
subl $4, %esp
movl %ebx, -36(%ebp)
# Push function call args
pushl 0
pushl $.LC13
pushl $.LC8
# Function call
call func
subl $4, %esp
movl %ebx, -40(%ebp)
# Add value to stack
subl $4, %esp
movl $.LC8, -44(%ebp)
# Push function call args
pushl $.LC8
# Function call
call print
subl $4, %esp
movl %ebx, -48(%ebp)
# Push function call args
pushl $.LC15
# Function call
call print
subl $4, %esp
movl %ebx, -52(%ebp)
# Assignment: Avoid too many memory references
movl -32(%ebp), %ecx
movl %ecx, -24(%ebp)
# Push function call args
pushl 0
pushl $.LC10
pushl $.LC7
# Function call
call func
subl $4, %esp
movl %ebx, -56(%ebp)
# Get function call return value: avoiding too many memory references
movl -56(%ebp), %ecx
# Assignment
movl %ecx, -44(%ebp)
# Push function call args
pushl $.LC10
# Function call
call print
subl $4, %esp
movl %ebx, -60(%ebp)
# Push function call args
pushl $.LC17
# Function call
call print
subl $4, %esp
movl %ebx, -64(%ebp)
# Return
movl 0, %ebx
leave
ret

