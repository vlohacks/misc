#include "pci.h"
#include "types.h"
#include "util.h"

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

void pci_enum() 
{
	uint16_t bus;
	uint8_t slot;
	uint32_t tmp, tmp2;
	
	char buf[32];
	
	for (bus = 0; bus < 256; bus++) {
		for (slot = 0; slot < 32; slot++) {
			tmp = pci_config_read(bus, slot, 0x00, 0x00);
			// empty slot?
			if ((tmp & 0xffff) == 0xffff)
				continue;
			tmp2 = pci_config_read(bus, slot, 0x00, 0x08);
			
			vk_printf("PCI   : %02x:%02x vendor=%04x, device=%04x, class=%02x, subclass=%02x\n", bus, slot, tmp & 0xffff, tmp >> 16, tmp2 >> 24, (tmp2 >> 16) & 0xff);
		}
	}
		
	
}

