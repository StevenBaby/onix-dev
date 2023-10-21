#include <onix/stdarg.h>
#include <onix/printk.h>

static char buf[1024];

err_t printk(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    console_write(NULL, buf, i);
    return i;
}
