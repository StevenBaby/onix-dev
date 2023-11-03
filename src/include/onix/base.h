#pragma once

#include <onix/types.h>

namespace arch
{
    struct base_t
    {
        inline void *operator new(size_t, void *ptr) { return ptr; }
    };
}