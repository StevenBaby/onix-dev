#include <onix/types.h>
#include <onix/io.h>
#include <onix/device.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

_extern void i386_init()
{
    device::device_init();
    device::none_init();
    device::console_init();
    device::serial_init();

    LOGK("onix running in protected mode...\n");
    panic("panic....");
}