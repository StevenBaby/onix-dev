
#include <onix/types.h>
#include <onix/io.h>

void kernel_init()
{
    u32 data = inb(0x92);
    outb(0x92, (u8)data);

    data = inw(0xCFC);
    outw(0xCFC, (u16)data);

    data = inl(0xCFC);
    outl(0xCFC, data);
}
