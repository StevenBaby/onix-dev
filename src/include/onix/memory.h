#ifndef ONIX_MEMORY_H
#define ONIX_MEMORY_H

#include <onix/types.h>

#define PAGE_SIZE 0x1000L

#define MEMORY_LOADER 0x1000L
#define MEMORY_PAGING 0x2000L // 0x2000 ~ 0x5000
#define MEMORY_BASEZONE 0x6000L
#define MEMORY_BASE 0x100000L

#define IDX(addr) (((u64)addr) >> 12)
#define PIDX1(addr) ((((u64)addr) >> 39) & 0x1ff)
#define PIDX2(addr) ((((u64)addr) >> 30) & 0x1ff)
#define PIDX3(addr) ((((u64)addr) >> 21) & 0x1ff)
#define PIDX4(addr) ((((u64)addr) >> 12) & 0x1ff)
#define PIDX(addr, level) ((((u64)addr) >> (12 + (4 - (level)) * 9)) & 0x1ff)
#define PAGE(idx) (((u64)idx) << 12)

typedef struct page_entry_t
{
    u8 present : 1;    // in memory
    u8 write : 1;      // 0 readonly 1 writeable
    u8 user : 1;       // 1 all user 0 super user DPL < 3
    u8 pwt : 1;        // page write through 1 write direct, 0 write back
    u8 pcd : 1;        // page cache disable
    u8 accessed : 1;   // accessed
    u8 dirty : 1;      // different with cache
    u8 pat : 1;        // page attribute table 4K/4M
    u8 global : 1;     // all task used this page
    u8 ignored : 3;    // ignored
    u64 index : 40;    // index
    u32 RESERVED : 11; // reserved
    u8 xd : 1;         // execute
} _packed page_entry_t;

typedef struct page_t
{
    u32 count; // 引用计数
} page_t;

// Address Range Descriptor Structure
typedef struct ards_t
{
    u64 base; // memory base
    u64 size; // memory size
    u32 type; // memory type
} _packed ards_t;

#define ARDS_TABLE_LEN 32
extern ards_t ards_table[ARDS_TABLE_LEN];
extern u32 ards_count;

extern u32 kernel_chksum;
extern u32 kernel_size;

#define ARDS_ZONE_VALID 1 // ards 可用内存区域

void map_page(u64 vaddr, u64 paddr);
void map_area(u64 paddr, u64 size);
ards_t *memory_area(u64 addr);

#endif