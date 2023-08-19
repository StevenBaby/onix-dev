BUILD:=../build
SRC:=.

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD)

include $(SRC)/utils/image.mk
include $(SRC)/utils/bochs.mk
include $(SRC)/utils/qemu.mk
include $(SRC)/utils/efi.mk
