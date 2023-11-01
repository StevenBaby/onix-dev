#include <onix/types.h>
#include <onix/device.h>
#include <onix/test.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

_extern void x64_init()
{
    device::device_init();
    device::none_init();
    device::console_init();
    device::serial_init();

    LOGK("onix running in long mode...\n");

    test_init();
}
