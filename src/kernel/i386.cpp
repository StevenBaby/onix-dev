#include <onix/device.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

namespace arch
{
    _extern void i386_init()
    {
        device::device_init();
        device::none_init();
        device::console_init();
        LOGK("onix running in protected mode...\n");
        panic("panic....");
    }
}
