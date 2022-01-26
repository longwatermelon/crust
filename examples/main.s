.section .data
.LC0: .asciz "    \n"
.LC1: .asciz "string"
.LC3: .asciz "bbbbb"
.LC2: .asciz "aaaaa"
.LC4: .asciz "   \n\n"
.LC5: .asciz "aaaaa"
.LC6: .asciz "bbbbb"
.LC7: .asciz "aaaaa"
.LC8: .asciz "bbbbb"
.LC9: .asciz "bbbbb"
.LC10: .asciz "    \n"
.LC11: .asciz " k  \n"
.globl _start
_start:
call main
mov $1, %eax
int $0x80
.globl func
func:
pushl %ebp
movl %esp, %ebp
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl 8(%ebp), %ecx
int $0x80
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl 12(%ebp), %ecx
int $0x80
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl $.LC0, %ecx
int $0x80
movl 12(%ebp), %ebx
leave
ret
.globl test
test:
pushl %ebp
movl %esp, %ebp
leave
ret
.globl main
main:
pushl %ebp
movl %esp, %ebp
subl $4, %esp
movl $1, -4(%ebp)
subl $4, %esp
movl $.LC1, 0(%ebp)
pushl $0
pushl $.LC3
pushl $.LC2
call func
subl $4, %esp
movl %ebx, -4(%ebp)
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl $.LC4, %ecx
int $0x80
subl $4, %esp
movl $.LC5, -12(%ebp)
subl $4, %esp
movl $.LC6, -16(%ebp)
subl $4, %esp
movl $9, -20(%ebp)
subl $4, %esp
movl $.LC7, -24(%ebp)
pushl $0
pushl $.LC8
pushl $.LC5
call func
subl $4, %esp
movl %ebx, -28(%ebp)
pushl $0
pushl $.LC9
pushl $.LC6
call func
subl $4, %esp
movl %ebx, -32(%ebp)
subl $4, %esp
movl $.LC6, -36(%ebp)
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl -36(%ebp), %ecx
int $0x80
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl $.LC10, %ecx
int $0x80
movl -24(%ebp), %ecx
movl %ecx, -16(%ebp)
pushl $0
pushl $.LC7
pushl $.LC5
call func
subl $4, %esp
movl %ebx, -44(%ebp)
movl -44(%ebp), %ecx
movl %ecx, -36(%ebp)
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl -36(%ebp), %ecx
int $0x80
movl $5, %edx
movl $1, %ebx
movl $4, %eax
movl $.LC11, %ecx
int $0x80
movl $0, %ebx
leave
ret

