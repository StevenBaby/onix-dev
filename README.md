# onix-dev

fast forward testing for onix operating system

## References

### bootloader

read disk with bios

- https://en.wikipedia.org/wiki/INT_13H
- https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)#LBA_in_Extended_Mode

a20 line

- https://wiki.osdev.org/A20_Line

detect memory

- https://wiki.osdev.org/Detecting_Memory_(x86)

multiboot2

    sudo pacman -S grub
    sudo pacman -S xorriso
    sudo pacman -S mtools

### gcc options

    sudo pacman -S lib32-glibc

- https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html

### assembly language

- https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html

### SMP

- https://www.qemu.org/docs/master/system/qemu-manpage.html
- https://bochs.sourceforge.io/doc/docbook/user/smp.html
- https://bochs.sourceforge.io/doc/docbook/user/bochsrc.html#BOCHSOPT-CPU

acpi timer

- https://wiki.osdev.org/APIC_Timer
