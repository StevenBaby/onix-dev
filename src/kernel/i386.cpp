#include <onix/types.h>
#include <onix/io.h>
#include <onix/device.h>

_extern void i386_init()
{
    device::device_init();
    device::none_init();
    device::console_init();
}