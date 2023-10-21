#ifndef ONIX_IO_H
#define ONIX_IO_H

#include <onix/types.h>

u8 inb(u16 port);  // in byte
u16 inw(u16 port); // in word
u32 inl(u16 port); // in double word long

void outb(u16 port, u8 data);  // out byte
void outw(u16 port, u16 data); // out word
void outl(u16 port, u32 data); // out double word long

#endif