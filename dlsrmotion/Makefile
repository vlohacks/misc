
NAME := dlsrmotion
HEX := $(NAME).hex
OUT := $(NAME).elf
MAP := $(NAME).map

OBJ = dlsrmotion.o

MCU := attiny13a
MCU_AVRDUDE := t13
MCU_FREQ := 128000UL
             
CC := avr-gcc
OBJCOPY := avr-objcopy
SIZE := avr-size -A

CFLAGS := -L /usr/lib/avr/lib/avr25/ -Wall -mmcu=$(MCU) -Os -std=c99 -DF_CPU=$(MCU_FREQ) 

all: $(HEX)

clean:
	rm -f $(HEX) $(OUT) $(MAP) $(OBJ)

# -B512 = slow bitclock bcoz of slow mcu freq
flash: $(HEX)
	avrdude -y -c avrispmkII -B512 -P usb -p $(MCU_AVRDUDE) -U flash:w:$(HEX) -U lfuse:w:0x7b:m -U hfuse:w:0xff:m 

$(HEX): $(OUT)
	$(OBJCOPY) -R .eeprom -O ihex $< $@

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ -Wl,-Map,$(MAP) $^
	@echo
	@$(SIZE) $@
	@echo

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.sx
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: all clean flash doc

