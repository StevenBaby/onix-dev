#ifndef ONIX_STDLIB_H
#define ONIX_STDLIB_H

#include <onix/types.h>

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

#define ALIGN(addr, bytes) (((addr) + (bytes - 1)) & ~(bytes - 1))

// 计算 num 分成 size 的数量
u64 div_round_up(u64 num, u64 size);

u8 bcd_to_bin(u8 value);
u8 bin_to_bcd(u8 value);

#endif