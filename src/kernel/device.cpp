
#include <onix/device.h>
#include <onix/string.h>

#define DEVICE_NUM 64

using namespace arch::device;

static device_t *device_table[DEVICE_NUM];

int device_t::read(char *data, size_t size)
{
    return 0;
}

int device_t::write(char *data, size_t size)
{
    return 0;
}

void device::device_init()
{
    memset(device_table, 0, sizeof(device_table));
}

device_t *device::instance(device_type type, int idx)
{
    int cnt = 0;
    for (device_t *dev : device_table)
    {
        if (dev == nullptr || dev->type != type)
            continue;
        if (cnt == idx)
            return dev;
        cnt++;
    }
    return nullptr;
}

void device::install(device_t *dev)
{
    for (size_t i = 0; i < DEVICE_NUM; i++)
    {
        if (device_table[i] != nullptr)
            continue;
        device_table[i] = dev;
        return;
    }
}
