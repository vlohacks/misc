KERNEL_DIR		= kernel
TESTTASK_DIR	= testtask

.PHONY: kernel
.PHONY: testtask


kernel: 
	$(MAKE) -C $(KERNEL_DIR)
	cp $(KERNEL_DIR)/kernel.bin .
	
testtask:
	$(MAKE) -C $(TESTTASK_DIR)
	cp $(TESTTASK_DIR)/*.elf .

iso: kernel testtask
	cp kernel.bin iso/boot/
	./setup_iso.sh	
	grub-mkrescue -d /usr/lib/grub/i386-pc/ -o image.iso iso/

run: kernel testtask iso
	qemu-system-i386 -D ./qemu.log -d int,cpu_reset -soundhw ac97 -cdrom image.iso
	#qemu-system-i386 -kernel kernel.bin

clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	$(MAKE) -C $(TESTTASK_DIR) clean
	rm -f kernel.bin
	rm -f *.elf
	rm -f iso/boot/kernel.bin
	rm -f image.iso
