.section .data
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

