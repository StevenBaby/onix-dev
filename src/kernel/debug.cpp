#include <onix/debug.h>
#include <onix/device.h>
#include <onix/debug.h>
#include <onix/stdarg.h>
#include <onix/printk.h>

namespace arch
{
    static char buf[1024];

    void debugk(const char *file, int line, const char *fmt, ...)
    {
        device::device_t *ptr = device::instance(device::DEVICE_SERIAL);

        int i = sprintf(buf, "[%s] [%d] ", file, line);
        ptr->write(buf, i);

        va_list args;
        va_start(args, fmt);
        i = vsprintf(buf, fmt, args);
        va_end(args);
        ptr->write(buf, i);
    }
}