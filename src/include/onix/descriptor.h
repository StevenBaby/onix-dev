#ifndef ONIX_GLOBAL_H
#define ONIX_GLOBAL_H

#include <onix/types.h>

#define KERNEL_CODE_IDX 1
#define KERNEL_DATA_IDX 2

// descriptor
typedef struct descriptor_t
{
    unsigned short limit_low;      // segment limit 0 ~ 15 bits
    unsigned int base_low : 24;    // segment base 0 ~ 23 bits
    unsigned char type : 4;        // segment type
    unsigned char segment : 1;     // 1 code or data，0 system segment
    unsigned char DPL : 2;         // Descriptor Privilege Level 0 ~ 3
    unsigned char present : 1;     // 1 in memory，0 no exists
    unsigned char limit_high : 4;  // segment limit 16 ~ 19 bits
    unsigned char available : 1;   // arranged everything. Given to the OS
    unsigned char long_mode : 1;   // long mode
    unsigned char big : 1;         // 32 bit or 16 bit
    unsigned char granularity : 1; // 4KB or 1B
    unsigned char base_high;       // segment base 24 ~ 31 bits
} _packed descriptor_t;

// memory pointer
typedef struct pointer_t
{
    u16 limit;
    u64 base;
} _packed pointer_t;

#endif