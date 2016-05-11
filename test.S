	.equ Testsymbol, 60
	.equ testSymbol, 100500
	.equ testSymbol, 1005

	.section .data
sprsayan:
	.ascii "IT'S OVER NINE THOUSAAAAAAND!\0"
shrthxdt:
	.short 0x01Ab
bytbindt:
	.byte 0b01001011
longdcdt:
	.long 100500

	.globl _start
	.section .text
_start:
	cmpsd
	jbe _testend
	mulq %rbx
	decl longdcdt
	addl %eax, %ecx
	jbe _testend
	cmpq shrthxdt, %rdx
	andq %rax, shrthxdt
	shlq $3, %rcx
	orl $0xF187DD16, longdcdt
	jbe _start
_testend:

	.if 1

	decq shrthxdt(%rsi,%rdi)
	decl (%rdi,%rsi)

	.else

	decl shrthxdt(%rsi,%rdi)
	decq (%rdi,%rsi)

	.endif
