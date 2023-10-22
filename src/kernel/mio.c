#include <onix/mio.h>

u8 minb(u64 addr)
{
    return *((volatile u8 *)addr);
}

u16 minw(u64 addr)
{
    return *((volatile u16 *)addr);
}

u32 minl(u64 addr)
{
    return *((volatile u32 *)addr);
}

u64 minq(u64 addr)
{
    return *((volatile u64 *)addr);
}

void moutb(u64 addr, u8 value)
{
    *((volatile u8 *)addr) = value;
}

void moutw(u64 addr, u16 value)
{
    *((volatile u16 *)addr) = value;
}

void moutl(u64 addr, u32 value)
{
    *((volatile u32 *)addr) = value;
}

void moutq(u64 addr, u64 value)
{
    *((volatile u64 *)addr) = value;
}
