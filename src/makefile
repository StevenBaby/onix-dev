BUILD:=../build
SRC:=.

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/harddisk.img: \
	$(BUILD)/boot/boot.bin \

	# 创建一个 16M 的硬盘镜像
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@

	# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc

IMAGES:= $(BUILD)/harddisk.img

image: $(IMAGES)

.PHONY: clean
clean:
	rm -rf $(BUILD)

include $(SRC)/utils/bochs.mk
include $(SRC)/utils/qemu.mk
