
EFI_GGC:=x86_64-w64-mingw32-gcc

EFI_CFLAGS:=
EFI_CFLAGS+= -Wall
EFI_CFLAGS+= -Wextra
EFI_CFLAGS+= -nostdinc
EFI_CFLAGS+= -nostdlib
EFI_CFLAGS+= -fno-builtin
EFI_CFLAGS+= -Wl,--subsystem,10 # 向连接器传递参数，系统类型 efi，见 UEFI 2.1 Boot Manger
# EFI_CFLAGS+= -v

$(BUILD)/esp/EFI/BOOT/BOOTX64.EFI: $(SRC)/efi/efi.c
	$(shell mkdir -p $(dir $@))
	$(EFI_GGC) $(EFI_CFLAGS) -e efi_main $< -o $@

.PHONY: efi
efi: $(BUILD)/esp/EFI/BOOT/BOOTX64.EFI

QEMU:= qemu-system-x86_64

OVMFCODE=/usr/share/OVMF/x64/OVMF_CODE.fd
OVMFVARS=/usr/share/OVMF/x64/OVMF_VARS.fd

QEMU_EFI_FLAGS:= -net none
QEMU_EFI_FLAGS+= -drive if=pflash,format=raw,readonly=on,file=$(OVMFCODE) 
QEMU_EFI_FLAGS+= -drive if=pflash,format=raw,readonly=on,file=$(OVMFVARS) 
QEMU_EFI_FLAGS+= -drive file=fat:rw:$(BUILD)/esp,index=0,format=vvfat

.PHONY: qemuefi
qemuefi: $(BUILD)/esp/EFI/BOOT/BOOTX64.EFI
	$(QEMU) $(QEMU_EFI_FLAGS)


USBDEV:= /dev/sdb1

.PHONY: usbefi
usbefi: $(BUILD)/esp/EFI/BOOT/BOOTX64.EFI $(USBDEV)
	sudo mount $(USBDEV) /mnt
	sudo mkdir -p /mnt/EFI/BOOT
	sudo cp $< /mnt/EFI/BOOT/BOOTX64.EFI
	sudo umount /mnt
