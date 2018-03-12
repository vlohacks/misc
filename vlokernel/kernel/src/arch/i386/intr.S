// macros for generating interrupt stub entries

.macro intr_stub nr
.global intr_stub_\nr
intr_stub_\nr:
    push $0
    push $\nr
    jmp intr_common_handler
.endm
 
.macro intr_stub_error_code nr
.global intr_stub_\nr
intr_stub_\nr:
    push $\nr
    jmp intr_common_handler
.endm
 
// Exceptions
intr_stub 0
intr_stub 1
intr_stub 2
intr_stub 3
intr_stub 4
intr_stub 5
intr_stub 6
intr_stub 7
intr_stub_error_code 8
intr_stub 9
intr_stub_error_code 10
intr_stub_error_code 11
intr_stub_error_code 12
intr_stub_error_code 13
intr_stub_error_code 14
intr_stub 15
intr_stub 16
intr_stub_error_code 17
intr_stub 18
 
// IRQs
intr_stub 32
intr_stub 33
intr_stub 34
intr_stub 35
intr_stub 36
intr_stub 37
intr_stub 38
intr_stub 39
intr_stub 40
intr_stub 41
intr_stub 42
intr_stub 43
intr_stub 44
intr_stub 45
intr_stub 46
intr_stub 47
 
// Syscall
intr_stub 48


.extern handle_interrupt
intr_common_handler:

	// save current CPU state
	push %ebp
	push %edi
	push %esi
	push %edx
	push %ecx
	push %ebx
	push %eax

	// load kernel DS and ES
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es


	// call interrupt handler in C
	push %esp
	call interrupt_handle
	
	// realign stack to possibly new CPU state
	// interrupt_handle might have returned a new state
	// ie due to scheduling 
	mov %eax, %esp

	// load user DS and ES
	mov $0x23, %ax
 	mov %ax, %ds
	mov %ax, %es

	// restore CPU state
	pop %eax
	pop %ebx
	pop %ecx
	pop %edx
	pop %esi
	pop %edi
	pop %ebp
 
	// remove error code and interrupt number from stack
	// which were pushed by interrupt stub handlers (see above)
	add $8, %esp

	// return to interrupted code
	iret
