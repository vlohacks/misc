#!/bin/bash
GRUBCFG="iso/boot/grub/grub.cfg"
echo 'menuentry "vloOS" {' > $GRUBCFG
echo '	multiboot /boot/kernel.bin' >> $GRUBCFG
for I in *.elf; do 
	cp $I iso/boot/$I
	echo "	module /boot/$I $I" >> $GRUBCFG 
done
echo '}' >> $GRUBCFG
