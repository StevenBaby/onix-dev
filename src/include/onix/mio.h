#ifndef ONIX_MIO_H
#define ONIX_MIO_H

#include <onix/types.h>

// 映射内存 IO

u8 minb(u64 addr);
u16 minw(u64 addr);
u32 minl(u64 addr);
u64 minq(u64 addr);


void moutb(u64 addr, u8 value);
void moutw(u64 addr, u16 value);
void moutl(u64 addr, u32 value);
void moutq(u64 addr, u64 value);

#endif
