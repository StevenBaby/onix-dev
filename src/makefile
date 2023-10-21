BUILD:=../build
SRC:=.

# i386=1

MULTIBOOT2:=0x18000
ENTRYPOINT:=$(shell python -c "print(f'0x{$(MULTIBOOT2) + 0x8000:x}')")
ONIX_MAGIC:= 0x20220205

ifdef i386
CFLAGS += -m32
endif

CFLAGS+= -fno-builtin # no builtin functions
# CFLAGS+= -nostdinc # no standard include files
CFLAGS+= -fno-pic # no position independent code
CFLAGS+= -fno-pie # no position independent executable
CFLAGS+= -nostdlib # no standard library
CFLAGS+= -fno-stack-protector # no stack protect
CFLAGS:=$(strip ${CFLAGS})

DEBUG:= -g
INCLUDE:=-I$(SRC)/include

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

LOADER_FLAGS:=
LOADER_FLAGS+= -DKERNEL_SIZE=$$(stat -c%s "$(BUILD)/system.bin")
LOADER_FLAGS+= -DTARGET_MEMORY=$(MULTIBOOT2)
LOADER_FLAGS+= -DENTRYPOINT=$(ENTRYPOINT)
LOADER_FLAGS+= -DONIX_MAGIC=$(ONIX_MAGIC)

$(BUILD)/boot/loader.bin: $(SRC)/boot/loader.asm $(BUILD)/system.bin
	$(shell mkdir -p $(dir $@))
	nasm $(LOADER_FLAGS) -f bin $< -o $@ 

ifdef i386
NASM_FLAGS += -f elf32
else
NASM_FLAGS += -f elf64
endif

$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm $(NASM_FLAGS) $(DEBUG) $< -o $@

$(BUILD)/%32.o: $(SRC)/%32.c
	$(shell mkdir -p $(dir $@))
	gcc -m32 $(CFLAGS) $(DEBUG) $(INCLUDE) -S $< -o $@.s
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $@.s -o $@

$(BUILD)/%.o: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

SFLAGS:= -fno-asynchronous-unwind-tables
SFLAGS+= -fno-dwarf2-cfi-asm

$(BUILD)/%.s: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(INCLUDE) $(SFLAGS) -S $< -o $@

ifdef i386
LDFLAGS += -m elf_i386
endif
LDFLAGS+= -static
LDFLAGS+= -Ttext $(ENTRYPOINT)
LDFLAGS+= -section-start=.multiboot2=$(MULTIBOOT2)
LDFLAGS:=$(strip ${LDFLAGS})

$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/setup32.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/serial.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/vsprintf.o \

	$(shell mkdir -p $(dir $@))
	ld ${LDFLAGS} $^ -o $@

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary --set-section-flags .bss=alloc,load,contents $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/harddisk.img: \
	$(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map \

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

	python $(SRC)/utils/chksum.py

IMAGES:= $(BUILD)/harddisk.img

image: $(IMAGES)

.PHONY: clean
clean:
	rm -rf $(BUILD)

include $(SRC)/utils/bochs.mk
include $(SRC)/utils/qemu.mk
include $(SRC)/utils/cdrom.mk
