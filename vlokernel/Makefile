KERNEL_DIR	= kernel

.PHONY: kernel



kernel: 
	$(MAKE) -C $(KERNEL_DIR)
	cp $(KERNEL_DIR)/kernel.bin .

iso: kernel
	cp kernel.bin iso/boot/
	grub-mkrescue -o image.iso iso/

run: kernel
	qemu-system-i386 -soundhw ac97 -kernel kernel.bin
	#qemu-system-i386 -kernel kernel.bin

clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	rm -f kernel.bin
	rm -f iso/boot/kernel.bin
	rm -f image.iso
