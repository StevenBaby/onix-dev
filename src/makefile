BUILD:=../build
SRC:=.

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/harddisk.img: \
	$(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \

	# create a harddisk image with 16M capacity 
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@

	# write boot.bin to master boot sector
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc

	# write loader.bin to harddisk a page
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=8 seek=2 conv=notrunc

IMAGES:= $(BUILD)/harddisk.img

image: $(IMAGES)

.PHONY: clean
clean:
	rm -rf $(BUILD)

include $(SRC)/utils/bochs.mk
include $(SRC)/utils/qemu.mk
