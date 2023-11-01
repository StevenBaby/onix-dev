#include <onix/device.h>
#include <onix/string.h>

namespace arch::device
{
    struct none_t : device_t
    {
        int read(char *data, size_t size)
        {
            return 0;
        }

        int write(char *data, size_t size)
        {
            return 0;
        }
    };
}

static device::none_t none;

void device::none_init()
{
    device::install(new (&none) none_t());
}