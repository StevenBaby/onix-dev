
#include <onix/debug.h>
#include <onix/stdarg.h>
#include <onix/printk.h>

static char buf[1024];

void debugk(char *file, int line, const char *fmt, ...)
{

    err_t i = sprintf(buf, "[%s] [%d] ", file, line);
    serial_write(NULL, buf, i);

    va_list args;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    serial_write(NULL, buf, i);
}