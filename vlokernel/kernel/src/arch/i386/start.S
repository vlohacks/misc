# Multiboot declarations (constants)
.set MB_ALIGN,		1<<0
.set MB_MEMINFO, 	1<<1
.set MB_FLAGS,		MB_ALIGN | MB_MEMINFO
.set MB_MAGIC,		0x1badb002
.set MB_CHECKSUM,	-(MB_MAGIC + MB_FLAGS)


# Multiboot section
.section .multiboot
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM


# Initial stack
.section .initstack, "aw", @nobits
initstack_bottom:
.skip 16384
initstack_top:


# Entry point
.section .text
.global _start
.type _start, @function

_start:
	# set up stack pointer and enter kernel
	movl	$initstack_top, %esp
	
	# muliboot info in ebx as argument for kernel
	push	%ebx

	# call kernel C code
	call	kernel_main

	# clear interrupts and hang if kernel exits, this neven should happen
	cli
	hlt

init_loop:
	jmp init_loop	



.size _start, . - _start
