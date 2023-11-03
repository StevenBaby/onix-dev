#include <onix/io.h>

namespace arch::io
{
    // in byte
    u8 inb(u16 port)
    {
        u8 ret;
        asm volatile(
            "inb %%dx, %%al\n"
            : "=a"(ret)
            : "d"(port));
        return ret;
    }

    // in word
    u16 inw(u16 port)
    {
        u16 ret;
        asm volatile(
            "inw %%dx, %%ax\n"
            : "=a"(ret)
            : "d"(port));
        return ret;
    }

    // in double word long
    u32 inl(u16 port)
    {
        u32 ret;
        asm volatile(
            "inl %%dx, %%eax\n"
            : "=a"(ret)
            : "d"(port));
        return ret;
    }

    // out byte
    void outb(u16 port, u8 data)
    {
        asm volatile(
            "outb %%al, %%dx\n"
            : : "d"(port), "a"(data));
    }

    // out word
    void outw(u16 port, u16 data)
    {
        asm volatile(
            "outw %%ax, %%dx\n"
            : : "d"(port), "a"(data));
    }

    // out double word long
    void outl(u16 port, u32 data)
    {
        asm volatile(
            "outl %%eax, %%dx\n"
            : : "d"(port), "a"(data));
    }
}