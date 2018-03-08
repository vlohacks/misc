TOOLCHAIN	= /home/vlo/opt/cross/bin
CC		= i686-elf-gcc
AS		= i686-elf-as
OBJ		= src/arch/i386/intr.o src/arch/i386/start.o src/arch/i386/cpu_state.o src/arch/i386/exception.o src/arch/i386/gdt.o src/arch/i386/interrupt.o src/arch/i386/term.o src/arch/i386/pci.o src/main.o src/pmm.o src/sched.o src/task.o src/testtasks.o src/util.o
CCFLAGS		= -O2 -Wall -Wextra -Iinclude
BIN		= kernel.bin

all: $(OBJ)
	$(TOOLCHAIN)/$(CC) $(CCFLAGS) -T etc/kernel.ld -o $(BIN) $(OBJ) -nostdlib

	

.PHONY: clean
clean: 
	rm -f $(OBJ)
	rm $(BIN)

%.o: %.c
	$(TOOLCHAIN)/$(CC) $(CCFLAGS) -c -o $@ $^

%.o: %.S
	$(TOOLCHAIN)/$(AS) -o $@ $^
	
	

