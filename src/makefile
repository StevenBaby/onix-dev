BUILD:=../build
SRC:=.

ENTRYPOINT:= 0x10000

CFLAGS+= -fno-builtin # no builtin functions
CFLAGS+= -nostdinc # no standard include files
CFLAGS+= -fno-pic # no position independent code
CFLAGS+= -fno-pie # no position independent executable
CFLAGS+= -nostdlib # no standard library
CFLAGS+= -fno-stack-protector # no stack protect
CFLAGS:=$(strip ${CFLAGS})

DEBUG:= -g

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf64 $(DEBUG) $< -o $@

LDFLAGS:=
LDFLAGS+= -static
LDFLAGS+= -Ttext $(ENTRYPOINT)
LDFLAGS:=$(strip ${LDFLAGS})

$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \

	$(shell mkdir -p $(dir $@))
	ld ${LDFLAGS} $^ -o $@

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/harddisk.img: \
	$(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \

	# create a harddisk image with 16M capacity 
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@

	# write boot.bin to master boot sector
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc

	# write loader.bin to harddisk a page
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=8 seek=2 conv=notrunc

	# test system.bin size < 500K, otherwise must modify count in next dd command
	test -n "$$(find $(BUILD)/system.bin -size -500k)"

	# write system.bin to harddisk
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=1000 seek=10 conv=notrunc

IMAGES:= $(BUILD)/harddisk.img

image: $(IMAGES)

.PHONY: clean
clean:
	rm -rf $(BUILD)

include $(SRC)/utils/bochs.mk
include $(SRC)/utils/qemu.mk
