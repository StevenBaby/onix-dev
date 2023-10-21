#include <onix/types.h>

static _omit_frame code32()
{
    asm volatile(".code32\n");
}

char message[] = "Onix running in protected mode...";

void setup_long_mode(u32 magic, u32 *addr)
{
    char *video = (char *)0xb8000;
    for (int i = 0; i < sizeof(message); i++)
    {
        video[i * 2] = message[i];
    }
}