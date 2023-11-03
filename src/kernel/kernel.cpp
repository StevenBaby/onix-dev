#include <onix/kernel.h>
#include <onix/chksum.h>

extern u8 _end;
extern u8 _start;

kernel_t kernel = {
    0, // u32 chksum;
    0, // u32 size;
    (u32)&_start,
    (u32)&_end,
};
