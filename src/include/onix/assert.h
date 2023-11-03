#pragma once

#include <onix/types.h>

namespace arch
{
    void assertion_failure(const char *exp, char *file, char *base, int line);
    void panic(const char *fmt, ...);
}

#define assert(exp) \
    if (exp)        \
        ;           \
    else            \
        arch::assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
