#include <onix/printk.h>
#include <onix/device.h>

namespace arch
{
    static char buf[1024];

    int printk(const char *fmt, ...)
    {
        va_list args;
        int i;

        va_start(args, fmt);

        i = vsprintf(buf, fmt, args);

        va_end(args);

        device::device_t *ptr = device::instance(device::DEVICE_CONSOLE);
        ptr->write(buf, i);

        return i;
    }
}