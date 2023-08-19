
QEMU:= 
# QEMU+= qemu-system-x86_64 # 虚拟机
QEMU+= qemu-system-i386 # 虚拟机
QEMU+= -m 32M # 内存
QEMU+= -drive file=$(BUILD)/harddisk.img,if=ide,index=0,media=disk,format=raw # 主硬盘

QEMU_DISK_BOOT:=-boot c

QEMU_DEBUG:= -s -S

.PHONY: qemu
qemu: $(IMAGES)
	$(QEMU) $(QEMU_DISK_BOOT)

.PHONY: qemug
qemug: $(IMAGES)
	$(QEMU) $(QEMU_DISK_BOOT) $(QEMU_DEBUG)
