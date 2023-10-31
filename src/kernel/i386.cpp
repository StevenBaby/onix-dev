#include <onix/types.h>
#include <onix/io.h>

static const char message[] = "Onix running in protected mode...";

namespace i386
{
    _extern void i386_init()
    {
        u8 byte = io::inb(0x92);
        char *video = (char *)0xb8000;
        for (int i = 0; i < sizeof(message); i++)
        {
            video[i * 2] = message[i];
        }
    }
}
