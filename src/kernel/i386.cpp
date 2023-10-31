#include <onix/types.h>
#include <onix/io.h>
#include <onix/device.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/memory.h>
#include <onix/string.h>
#include <onix/stdlib.h>
#include <onix/multiboot2.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

ards_t _aligned(8) ards_table[ARDS_TABLE_LEN];
u32 ards_count = 0;

static void ards_init(u32 addr)
{
    ards_count = MIN(ARDS_TABLE_LEN, *(u32 *)addr);
    ards_t *ptr = (ards_t *)(addr + 4);
    memcpy(ards_table, ptr, ards_count * sizeof(ards_t));
}

static void multiboot2_ards_init(u32 addr)
{
    u32 size = *(unsigned int *)addr;
    multi_tag_t *tag = (multi_tag_t *)(addr + 8);

    while (tag->type != MULTIBOOT_TAG_TYPE_END)
    {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
            break;
        // 下一个 tag 对齐到了 8 字节
        tag = (multi_tag_t *)((u32)tag + ALIGN(tag->size, 8));
    }

    multi_tag_mmap_t *mtag = (multi_tag_mmap_t *)tag;
    multi_mmap_entry_t *entry = mtag->entries;
    while ((u32)entry < (u32)tag + tag->size)
    {
        memcpy(&ards_table[ards_count++], entry, sizeof(ards_t));
        entry = (multi_mmap_entry_t *)((u32)entry + mtag->entry_size);
    }
}

static void memory_init(u32 magic, u32 addr)
{
    printk("memory init i386 magic %#x addr %p\n", magic, addr);
    switch (magic)
    {
    case ONIX_MAGIC:
        ards_init(addr);
        break;
    case MULTIBOOT2_MAGIC:
        multiboot2_ards_init(addr);
        break;
    default:
        printk("Magic %#x unknown, init memory failure...\n", magic);
        while (true)
            ;
        break;
    }
}

_extern void i386_init(u32 magic, u32 addr)
{
    device::device_init();
    device::none_init();
    device::console_init();
    device::serial_init();

    printk("onix running in protected mode...\n");
    memory_init(magic, addr);
}