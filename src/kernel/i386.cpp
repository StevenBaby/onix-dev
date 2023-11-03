#include <onix/kernel.h>
#include <onix/chksum.h>
#include <onix/device.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

namespace arch
{
    void kernel_init()
    {
        u32 size = kernel.end - kernel.start;
        kernel.chksum = crc32((void *)kernel.start, size);
        kernel.size = size;
    }

    _extern void i386_init()
    {
        kernel_init();
        device::device_init();
        device::none_init();
        device::console_init();
        device::serial_init();

        LOGK("kernel chksum %#x size %d\n", kernel.chksum, kernel.size);
        panic("panic....");
    }
}
