#include "pci.h"
#include "types.h"
#include "term.h"

static inline uint32_t inl(uint16_t port)
{
	uint32_t result;
	asm volatile ("inl %1, %0" : "=a" (result) : "d" (port));
	return result;
}

static inline void outl(uint16_t port, uint32_t data) 
{
	asm volatile ("outl %0, %1" : : "a" (data), "d" (port));
}

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus	= (uint32_t)bus;
	uint32_t lslot  = (uint32_t)slot;
	uint32_t lfunc  = (uint32_t)func;
	
	uint32_t tmp = 0;
	
	address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
    outl(0xCF8, address);
    tmp = inl(0xCFC);
    return (tmp);
}

void pci_enum() {
	uint16_t bus;
	uint8_t slot;
	uint32_t tmp;
	
	char buf[32];
	
	for (bus = 0; bus < 256; bus++) {
		for (slot = 0; slot < 32; slot++) {
			tmp = pci_config_read(bus, slot, 0x00, 0x00);
			// empty slot?
			if ((tmp & 0xffff) == 0xffff)
				continue;
			
			term_puts("PCI " );
			
			itoa(buf, bus, 16, 2);
			
			term_puts(buf);
			term_putc(':');
			
			
			itoa(buf, slot, 16, 2);
			term_puts(buf);
			
			term_puts(" vendor=");	
			itoa(buf, tmp & 0xffff, 16, 4);
			term_puts(buf);
			
			term_puts(" device=");	
			itoa(buf, tmp >> 16, 16, 4);
			term_puts(buf);
			
			tmp = pci_config_read(bus, slot, 0x00, 0x08);

			term_puts(" class=");	
			itoa(buf, tmp >> 24, 16, 2);
			term_puts(buf);

			term_puts(" subclass=");	
			itoa(buf, ((tmp >> 16) & 0xff), 16, 2);
			term_puts(buf);
			
			term_putc('\n');
		}
	}
		
	
}

