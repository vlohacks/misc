TOOLCHAIN	= /home/vlo/opt/cross/bin

CC		= i686-elf-gcc
LD		= i686-elf-ld
STRIP	= i686-elf-strip

TESTPROGS = 1_printA 2_printB 3_fibonacci 4_badopcode 5_division0

ASFLAGS	= -m32
CFLAGS 	= -m32 -O2 -Wall -fno-stack-protector
LDFLAGS = -melf_i386 -Tetc/mbtask.ld

all: $(TESTPROGS)

$(TESTPROGS): %: src/%.c
	$(TOOLCHAIN)/$(CC) $(CFLAGS) -c -o $@.o $^
	$(TOOLCHAIN)/$(LD) $(LDFLAGS) -o $@.elf $@.o
	
clean:
	rm -f $(OBJS) $(TESTPROGS_BIN)

.PHONY: clean
