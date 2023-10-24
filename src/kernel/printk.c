#include <onix/stdarg.h>
#include <onix/printk.h>
#include <onix/spinlock.h>

static char buf[1024];

static spinlock_t lock;

err_t printk(const char *fmt, ...)
{
    va_list args;
    int i;

    spin_lock(&lock);

    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    console_write(NULL, buf, i);

    spin_unlock(&lock);
    return i;
}
