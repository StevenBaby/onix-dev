BUILD:=../build
SRC:=.

# i386=1

ifndef i386 
x64=1
endif

LOADER_ADDR:=0x1000

ENTRYPOINT:= 0x20000
ONIX_MAGIC:= 0x20220205

DEBUG:=-g

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -DLOADER_ADDR=$(LOADER_ADDR) -f bin $< -o $@

LOADER_FLAGS+= -DKERNEL_SIZE=$$(stat -c%s "$(BUILD)/system.bin")
LOADER_FLAGS+= -DLOADER_ADDR=$(LOADER_ADDR)
LOADER_FLAGS+= -DENTRYPOINT=$(ENTRYPOINT)
LOADER_FLAGS+= -DONIX_MAGIC=$(ONIX_MAGIC)

$(BUILD)/boot/loader.bin: $(SRC)/boot/loader.asm $(BUILD)/system.bin
	$(shell mkdir -p $(dir $@))
	nasm $(LOADER_FLAGS) $(DEBUG) -f bin $< -o $@ 

ifdef i386
NASM_FLAGS += -f elf32
else
NASM_FLAGS += -f elf64
endif

$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm $(NASM_FLAGS) $(DEBUG) $< -o $@

INCLUDE:=-I$(SRC)/include

ifdef i386
CPPFLAGS += -m32
endif

CPPFLAGS+= -fno-builtin # no builtin functions
# CFLAGS+= -nostdinc # no standard include files
CPPFLAGS+= -fno-pic # no position independent code
CPPFLAGS+= -fno-pie # no position independent executable
CPPFLAGS+= -nostdlib # no standard library
CPPFLAGS+= -fno-stack-protector # no stack protect
CPPFLAGS+= -std=c++23
CPPFLAGS+= -fno-exceptions
CPPFLAGS+= -fno-rtti


ifdef x64
AFLAGS+= -fno-asynchronous-unwind-tables
else
AFLAGS+= $(DEBUG)
endif

$(BUILD)/i386/%.o: $(SRC)/%.cpp
	$(shell mkdir -p $(dir $@))
	g++ -m32 $(CPPFLAGS) $(AFLAGS) $(INCLUDE) -S $< -o $@.s
	g++ $(CPPFLAGS) $(AFLAGS) $(INCLUDE) -c $@.s -o $@

$(BUILD)/x64/%.o: $(SRC)/%.cpp
	$(shell mkdir -p $(dir $@))
	g++ $(CPPFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

ifdef i386
LDFLAGS += -m elf_i386
endif

LDFLAGS+= -static
LDFLAGS+= -Ttext $(ENTRYPOINT)
LDFLAGS:=$(strip ${LDFLAGS})


I386_MODULES+= $(BUILD)/i386/kernel/i386.o
I386_MODULES+= $(BUILD)/i386/kernel/io.o
I386_MODULES+= $(BUILD)/i386/kernel/device.o
I386_MODULES+= $(BUILD)/i386/kernel/none.o
I386_MODULES+= $(BUILD)/i386/kernel/console.o
I386_MODULES+= $(BUILD)/i386/kernel/printk.o
I386_MODULES+= $(BUILD)/i386/lib/string.o
I386_MODULES+= $(BUILD)/i386/lib/arch64.o
I386_MODULES+= $(BUILD)/i386/lib/vsprintf.o

X64_MODULES+= $(BUILD)/x64/kernel/io.o
X64_MODULES+= $(BUILD)/x64/kernel/device.o
X64_MODULES+= $(BUILD)/x64/kernel/none.o
X64_MODULES+= $(BUILD)/x64/kernel/console.o
X64_MODULES+= $(BUILD)/x64/kernel/printk.o
X64_MODULES+= $(BUILD)/x64/lib/string.o
X64_MODULES+= $(BUILD)/x64/lib/vsprintf.o

KERNEL_MODULES+= $(BUILD)/kernel/start.o
KERNEL_MODULES+= ${I386_MODULES}
ifdef x64
KERNEL_MODULES+= ${X64_MODULES}
endif

$(BUILD)/kernel.bin: ${KERNEL_MODULES}

	$(shell mkdir -p $(dir $@))
	ld ${LDFLAGS} $^ -o $@

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy --set-section-flags .bss=alloc,load,contents  -O binary $< $@

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

IMAGES:= $(BUILD)/harddisk.img

image: $(IMAGES)

.PHONY: clean
clean:
	rm -rf $(BUILD)

include $(SRC)/utils/bochs.mk
include $(SRC)/utils/qemu.mk
