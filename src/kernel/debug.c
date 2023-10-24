
#include <onix/debug.h>
#include <onix/stdarg.h>
#include <onix/printk.h>
#include <onix/spinlock.h>
#include <onix/apic.h>

static char buf[1024];

static spinlock_t lock;

void debugk(char *file, int line, const char *fmt, ...)
{
    spin_lock(&lock);

    err_t i = sprintf(buf, "[%d] [%s] [%d] ", apic_local_id(), file, line);
    serial_write(NULL, buf, i);

    va_list args;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    serial_write(NULL, buf, i);

    spin_unlock(&lock);
}