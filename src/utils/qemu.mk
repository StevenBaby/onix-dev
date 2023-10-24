
ifdef i386
QEMU+= qemu-system-i386 # qemu virtual machine
else
QEMU+= qemu-system-x86_64 # qemu virtual machine
endif

QEMU+= -m 32M # memory
QEMU+= -rtc base=localtime # localtime
QEMU+= -drive file=$(BUILD)/harddisk.img,if=ide,index=0,media=disk,format=raw # harddisk
QEMU+= -chardev stdio,mux=on,id=com1 # char device 1
QEMU+= -serial chardev:com1 # serial 1
QEMU+= -smp sockets=1,cores=4,threads=1

QEMU_DISK_BOOT:=-boot c

QEMU_DEBUG:= -s -S # qemu debug

.PHONY: qemu
qemu: $(IMAGES)
	$(QEMU) $(QEMU_DISK_BOOT)

.PHONY: qemug
qemug: $(IMAGES)
	$(QEMU) $(QEMU_DISK_BOOT) $(QEMU_DEBUG)
