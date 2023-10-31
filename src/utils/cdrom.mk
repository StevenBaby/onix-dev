
$(BUILD)/kernel.iso : \
	$(BUILD)/kernel.bin \
	$(SRC)/utils/grub.cfg \

	# check kernel.bin is valid multiboot2 kernel
	grub-file --is-x86-multiboot2 $<

	# create iso directory
	mkdir -p $(BUILD)/iso/boot/grub
	
	# copy kernel.bin
	cp $< $(BUILD)/iso/boot

	# copy grub config
	cp $(SRC)/utils/grub.cfg $(BUILD)/iso/boot/grub

	# generate iso
	grub-mkrescue -o $@ $(BUILD)/iso

QEMU_CDROM := -drive file=$(BUILD)/kernel.iso,media=cdrom,if=ide # 光盘镜像

QEMU_CDROM_BOOT:= -boot d

.PHONY: qemu-cd
qemu-cd: $(BUILD)/kernel.iso $(IMAGES)
	$(QEMU) $(QEMU_CDROM) $(QEMU_CDROM_BOOT)

.PHONY: qemug-cd
qemug-cd: $(BUILD)/kernel.iso $(IMAGES)
	$(QEMU) $(QEMU_CDROM) $(QEMU_CDROM_BOOT) $(QEMU_DEBUG)

.PHONY:cdrom
cdrom: $(BUILD)/kernel.iso $(IMAGES)
	-
