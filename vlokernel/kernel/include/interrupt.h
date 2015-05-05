#ifndef _INTERRUPT_h
#define _INTERRUPT_H

#include "types.h"

struct idt_entry {
	uint16_t	base_lo;
	uint16_t	sel;
	uint8_t		reserved;
	uint8_t		flags;
	uint16_t	base_hi;
} __attribute__((packed));

struct idt_ptr {
	uint16_t 	limit;
	void*		ptr;
} __attribute__((packed));

#define IDT_NUM_ENTRIES	256

void interrupt_init(void);
void interrupt_enable(void);
void interrupt_disable(void);

extern void intr_stub_0();
extern void intr_stub_1();
extern void intr_stub_2();
extern void intr_stub_3();
extern void intr_stub_4();
extern void intr_stub_5();
extern void intr_stub_6();
extern void intr_stub_7();
extern void intr_stub_8();
extern void intr_stub_9();
extern void intr_stub_10();
extern void intr_stub_11();
extern void intr_stub_12();
extern void intr_stub_13();
extern void intr_stub_14();
extern void intr_stub_15();
extern void intr_stub_16();
extern void intr_stub_17();
extern void intr_stub_18();
extern void intr_stub_19();
extern void intr_stub_20();
extern void intr_stub_21();
extern void intr_stub_22();
extern void intr_stub_23();
extern void intr_stub_24();
extern void intr_stub_25();
extern void intr_stub_26();
extern void intr_stub_27();
extern void intr_stub_28();
extern void intr_stub_29();
extern void intr_stub_30();
extern void intr_stub_31();
extern void intr_stub_32();
extern void intr_stub_33();
extern void intr_stub_34();
extern void intr_stub_35();
extern void intr_stub_36();
extern void intr_stub_37();
extern void intr_stub_38();
extern void intr_stub_39();
extern void intr_stub_40();
extern void intr_stub_41();
extern void intr_stub_42();
extern void intr_stub_43();
extern void intr_stub_44();
extern void intr_stub_45();
extern void intr_stub_46();
extern void intr_stub_47();
extern void intr_stub_48();


#endif
