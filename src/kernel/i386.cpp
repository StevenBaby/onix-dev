#include <onix/device.h>

namespace arch
{
    _extern void i386_init()
    {
        device::device_init();
        device::none_init();
        device::console_init();
    }
}
