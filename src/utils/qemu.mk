
QEMU:= 
QEMU+= qemu-system-x86_64 # qemu virtual machine
QEMU+= -m 32M # memory
QEMU+= -drive file=$(BUILD)/harddisk.img,if=ide,index=0,media=disk,format=raw # harddisk

QEMU_DISK_BOOT:=-boot c

QEMU_DEBUG:= -s -S # qemu debug

.PHONY: qemu
qemu: $(IMAGES)
	$(QEMU) $(QEMU_DISK_BOOT)

.PHONY: qemug
qemug: $(IMAGES)
	$(QEMU) $(QEMU_DISK_BOOT) $(QEMU_DEBUG)
