#include "gdt.h"

static uint64_t gdt[GDT_NUM_ENTRIES];
static uint32_t tss[TSS_NUM_ENTRIES] = { 0, 0, 0x10 };


static void gdt_set_entry(int i, unsigned int base, unsigned int limit, int flags) {
	gdt[i] = limit & 0xffffLL;
	gdt[i] |= (base & 0xffffffLL) << 16;
	gdt[i] |= (flags & 0xffLL) << 40;
	gdt[i] |= ((limit >> 16) & 0xfLL) << 48;
	gdt[i] |= ((flags >> 8 )& 0xffLL) << 52;
	gdt[i] |= ((base >> 24) & 0xffLL) << 56;
}

static void gdt_load() {
	struct {
		uint16_t limit;
		void* ptr;
	} __attribute__((packed)) gdt_entries = {
		.limit = GDT_NUM_ENTRIES * 8 - 1,
		.ptr = gdt
	};
	
	asm volatile (
		"lgdt %0\n"
		"ljmpl $0x08, $1f\n"
		"1:\n"
		"movl $0x10, %%eax\n"
		"movl %%eax, %%ds\n"
		"movl %%eax, %%es\n"
		"movl %%eax, %%fs\n"
		"movl %%eax, %%gs\n"
		"movl %%eax, %%ss\n"
		: : "m" (gdt_entries): "eax"
	);

	// TSS laden
	asm volatile("ltr %%ax" : : "a" (5<<3));

}

void gdt_init() {
	gdt_set_entry(0, 0, 0, 0);
	gdt_set_entry(1, 0, 		0xfffff, 	GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT | GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
	gdt_set_entry(2, 0, 		0xfffff, 	GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT | GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
	gdt_set_entry(3, 0, 		0xfffff, 	GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT | GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
	gdt_set_entry(4, 0, 		0xfffff, 	GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT | GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
	gdt_set_entry(5, (uint32_t)tss,	sizeof(tss),	GDT_FLAG_TSS | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

	gdt_load();
}

void gdt_update_tss(uint32_t tss_esp0) {
	tss[1] = tss_esp0;
}


