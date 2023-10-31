#pragma once

#include <onix/types.h>

namespace arch::device
{
    enum device_type
    {
        DEVICE_NONE,
        DEVICE_CONSOLE,
        DEVICE_SERIAL,
    };

    struct device_t
    {
        device_type type;

        virtual int write(char *data, size_t size);
        virtual int read(char *data, size_t size);
    };

    void install(device_t *dev);
    device_t *instance(device_type type, int idx = 0);

    void device_init();
    void none_init();
    void console_init();
    void serial_init();
}
