
#include <onix/types.h>
#include <onix/stdlib.h>
#include <onix/memory.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

u64 memory_max_size = 0;

void memory_init()
{
    LOGK("memory init...\n");
    ards_t *ards = ards_table;
    for (size_t i = 0; i < ards_count; i++, ards++)
    {
        LOGK("Memory base 0x%p size 0x%p type %d\n", ards->base, ards->size, ards->type);
        if (ards->type == ARDS_ZONE_VALID)
        {
            memory_max_size = MAX(memory_max_size, ards->base + ards->size);
        }
    }

    LOGK("Memory max size 0x%p\n", memory_max_size);
    LOGK("kernel chksum 0x%08x\n", kernel_chksum);
}
