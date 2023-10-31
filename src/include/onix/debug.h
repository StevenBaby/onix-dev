#pragma once

#include <onix/types.h>

namespace arch
{
    void debugk(char *file, int line, const char *fmt, ...);
}

#define bochs_breakpoint() asm volatile("xchgw %bx, %bx") // bochs magic breakpoint

#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

// #define LOGK(fmt, args...) DEBUGK(fmt, ##args)
