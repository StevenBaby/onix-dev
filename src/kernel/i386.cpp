#include <onix/types.h>

static const char message[] = "Onix running in protected mode...";

namespace arch
{
    _extern void i386_init()
    {
        char *video = (char *)0xb8000;
        for (int i = 0; i < sizeof(message); i++)
        {
            video[i * 2] = message[i];
        }
    }
}
