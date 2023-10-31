#ifndef ONIX_MEMORY_H
#define ONIX_MEMORY_H

#include <onix/types.h>

#define PAGE_SIZE 0x1000L
#define MEMORY_LOADER 0x1000L
#define MEMORY_PAGING 0x2000L // 0x2000 ~ 0x5000
#define MEMORY_BASEZONE 0x6000L
#define MEMORY_BASE 0x100000L

// Address Range Descriptor Structure
typedef struct ards_t
{
    u64 base; // memory base
    u64 size; // memory size
    u32 type; // memory type
} _packed ards_t;

#define ARDS_ZONE_VALID 1 // ards 可用内存区域
#define ARDS_TABLE_LEN 32

extern ards_t ards_table[ARDS_TABLE_LEN];
extern u32 ards_count;

#endif