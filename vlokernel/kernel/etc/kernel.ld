ENTRY(_start)

SECTIONS {
	/* start @1M */
	. = 1M;

	kernel_start = .;
	/* Multiboot header and code */
	.text BLOCK(4K) : ALIGN(4K) {
		*(.multiboot)
		*(.text)
	}

	/* rw data */
	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}

	/* uninitialized rw data, stack */
	.bss BLOCK(4K) : ALIGN(4K) {
		*(COMMON)
		*(.bss)
		*(.initstack)
	}

	. = ALIGN(4K);
	kernel_end = .;
	
}
