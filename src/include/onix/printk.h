#pragma once

#include <onix/types.h>
#include <onix/stdarg.h>

namespace arch
{
    int vsprintf(char *buf, const char *fmt, va_list args);
    int sprintf(char *buf, const char *fmt, ...);
    int printk(const char *fmt, ...);
}
