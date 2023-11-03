#pragma once

#include <onix/types.h>

struct kernel_t
{
    u32 chksum;
    u32 size;
    u32 start;
    u32 end;
};

_extern kernel_t kernel;
