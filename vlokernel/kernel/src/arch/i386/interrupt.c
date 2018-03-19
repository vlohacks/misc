#include "interrupt.h"
#include "cpu_state.h"
#include "gdt.h"
#include "exception.h"
#include "testtasks.h"
#include "sched.h"
#include "term.h"
#include "pmm.h"

static struct idt_entry idt[IDT_NUM_ENTRIES];

static inline uint8_t inb(uint16_t port)
{
	uint8_t result;
	asm volatile ("inb %1, %0" : "=a" (result) : "Nd" (port));
	return result;
}

static inline void outb(uint16_t port, uint8_t data) 
{
	asm volatile ("outb %0, %1" : : "a" (data), "Nd" (port));
}

static void idt_setup_pic(void) 
{
	// Master-PIC initialisieren
	outb(0x20, 0x11); // Initialisierungsbefehl fuer den PIC
	outb(0x21, 0x20); // Interruptnummer fuer IRQ 0
	outb(0x21, 0x04); // An IRQ 2 haengt der Slave
	outb(0x21, 0x01); // ICW 4
 
	// Slave-PIC initialisieren
	outb(0xa0, 0x11); // Initialisierungsbefehl fuer den PIC
	outb(0xa1, 0x28); // Interruptnummer fuer IRQ 8
	outb(0xa1, 0x02); // An IRQ 2 haengt der Slave
	outb(0xa1, 0x01); // ICW 4
 
	// Alle IRQs aktivieren (demaskieren)
	outb(0x20, 0x0);
	outb(0xa0, 0x0);
}

static void idt_set_entry(int i, void * base, uint16_t sel, uint8_t flags) 
{
	idt[i].base_hi = ((uint32_t)base >> 16);
	idt[i].base_lo = ((uint32_t)base & 0xffff);
	idt[i].flags = flags;
	idt[i].sel = sel;
	idt[i].reserved = 0;
}

static void idt_load(void) 
{
	struct idt_ptr idtp = {
		.limit = IDT_NUM_ENTRIES * 8 - 1,
		.ptr = idt
	};
	
	asm volatile ("lidt %0" : : "m" (idtp));
}


void interrupt_init(void) 
{	
	int i;

	for (i=0; i<IDT_NUM_ENTRIES; i++) 
		idt_set_entry(0, 0, 0, 0);

	idt_set_entry(0, intr_stub_0, 0x08, 0x8e);
	idt_set_entry(1, intr_stub_1, 0x08, 0x8e);
	idt_set_entry(2, intr_stub_2, 0x08, 0x8e);
	idt_set_entry(3, intr_stub_3, 0x08, 0x8e);
	idt_set_entry(4, intr_stub_4, 0x08, 0x8e);
	idt_set_entry(5, intr_stub_5, 0x08, 0x8e);
	idt_set_entry(6, intr_stub_6, 0x08, 0x8e);
	idt_set_entry(7, intr_stub_7, 0x08, 0x8e);
	idt_set_entry(8, intr_stub_8, 0x08, 0x8e);
	idt_set_entry(9, intr_stub_9, 0x08, 0x8e);
	idt_set_entry(10, intr_stub_10, 0x08, 0x8e);
	idt_set_entry(11, intr_stub_11, 0x08, 0x8e);
	idt_set_entry(12, intr_stub_12, 0x08, 0x8e);
	idt_set_entry(13, intr_stub_13, 0x08, 0x8e);
	idt_set_entry(14, intr_stub_14, 0x08, 0x8e);
	idt_set_entry(15, intr_stub_15, 0x08, 0x8e);
	idt_set_entry(16, intr_stub_16, 0x08, 0x8e);
	idt_set_entry(17, intr_stub_17, 0x08, 0x8e);
	idt_set_entry(18, intr_stub_18, 0x08, 0x8e);

	idt_set_entry(32, intr_stub_32, 0x08, 0x8e);
	idt_set_entry(33, intr_stub_33, 0x08, 0x8e);
	idt_set_entry(34, intr_stub_34, 0x08, 0x8e);
	idt_set_entry(35, intr_stub_35, 0x08, 0x8e);
	idt_set_entry(36, intr_stub_36, 0x08, 0x8e);
	idt_set_entry(37, intr_stub_37, 0x08, 0x8e);
	idt_set_entry(38, intr_stub_38, 0x08, 0x8e);
	idt_set_entry(39, intr_stub_39, 0x08, 0x8e);
	idt_set_entry(40, intr_stub_40, 0x08, 0x8e);
	idt_set_entry(41, intr_stub_41, 0x08, 0x8e);
	idt_set_entry(42, intr_stub_42, 0x08, 0x8e);
	idt_set_entry(43, intr_stub_43, 0x08, 0x8e);
	idt_set_entry(44, intr_stub_44, 0x08, 0x8e);
	idt_set_entry(45, intr_stub_45, 0x08, 0x8e);
	idt_set_entry(46, intr_stub_46, 0x08, 0x8e);
	idt_set_entry(47, intr_stub_47, 0x08, 0x8e);

	idt_set_entry(48, intr_stub_48, 0x08, 0xee);

	idt_setup_pic();
	idt_load();
}


void interrupt_enable(void) 
{
	asm volatile ("sti");
}

void interrupt_disable(void) 
{
	asm volatile ("cli");
}

struct cpu_state * interrupt_handle(struct cpu_state * cpu) 
{
	int x;

	struct cpu_state * new_cpu = cpu;

	if (cpu->intr <= 0x1f) {
		// Exception
		exception_lmaa(cpu);
	
	} else {
		if (cpu->intr == 0x20) {
			new_cpu = sched_schedule(cpu);
			gdt_update_tss((uint32_t)(new_cpu + 1));
		} else if (cpu->intr == 0x21) {
			
			x = (int)inb(0x60);
			if (x == 1) 
				exception_fuck_system();
			
			if ((x >= 130) && (x <= 135)) {
				new_cpu = testtasks_toggle(x-130, cpu);
				gdt_update_tss((uint32_t)(new_cpu + 1));
			}
			
			if (x == 136) {
				pmm_show_bitmap(0, 64);
			}

			if (x == 137) {
				sched_ps();
			}

		} else if (cpu->intr == 48) {
			// Syscall
			new_cpu = syscall(cpu);
		} else {
			vk_printf("unhandled interrupt: 0x%02x", cpu->intr);
			panic("Unhandled interrupt");
		}
	}

	// HW Interrupt acknowledgen
	if (cpu->intr >= 0x20 && cpu->intr <= 0x2f) {
		if (cpu->intr >= 0x28) {
			outb(0xa0, 0x20);	// ack 2. pic
		}
		outb(0x20, 0x20); 		// ack 1. pic
	}

	return new_cpu;
}

