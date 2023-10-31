#include <onix/types.h>
#include <onix/io.h>
#include <onix/device.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/memory.h>
#include <onix/string.h>
#include <onix/stdlib.h>
#include <onix/descriptor.h>
#include <onix/multiboot2.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define MSR_IA32_EFER 0xC0000080

ards_t _aligned(8) ards_table[ARDS_TABLE_LEN];
u32 ards_count = 0;

#define GDT_SIZE 8
static _aligned(8) cpu::descriptor_t gdt[GDT_SIZE]; // global descriotpr table
static _aligned(8) cpu::pointer_t gdt_ptr;          // gdt pointer

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

static void gdt_init()
{
    LOGK("gdt init...\n");

    memset(gdt, 0, sizeof(gdt));

    cpu::descriptor_t *desc;
    desc = gdt + KERNEL_CODE_IDX;
    desc->segment = 1;   // code
    desc->long_mode = 1; // long mode
    desc->present = 1;   // in memory
    desc->DPL = 0;       // dpl
    desc->type = 0b1010; // code / n oconforming / readable / no access

    desc = gdt + KERNEL_DATA_IDX;
    desc->segment = 1;   // data
    desc->long_mode = 1; // long mode
    desc->present = 1;   // in memory
    desc->DPL = 0;       // DPL
    desc->type = 0b0010; // data / upward / writeable / noaccess

    gdt_ptr.base = (u32)gdt;
    gdt_ptr.limit = sizeof(gdt) - 1;

    asm volatile("lgdt %0\n" ::"m"(gdt_ptr));
}


static _inline void entry_init(page_entry_t *entry, u32 index)
{
    *(u64 *)entry = PAGE(index) | 7;
}

static void paging_init()
{
    LOGK("paging init...\n");

    memset((void *)MEMORY_PAGING, 0, PAGE_SIZE * 4);
    page_entry_t *entry;

    entry = (page_entry_t *)MEMORY_PAGING;
    entry_init(entry, IDX(MEMORY_PAGING + PAGE_SIZE));
    entry_init(&entry[511], IDX(MEMORY_PAGING));

    entry = (page_entry_t *)(MEMORY_PAGING + PAGE_SIZE);
    entry_init(entry, IDX(MEMORY_PAGING + PAGE_SIZE * 2));

    entry = (page_entry_t *)(MEMORY_PAGING + PAGE_SIZE * 2);
    entry_init(entry, IDX(MEMORY_PAGING + PAGE_SIZE * 3));

    entry = (page_entry_t *)(MEMORY_PAGING + PAGE_SIZE * 3);
    for (size_t i = 1; i < 512; i++)
    {
        entry_init(entry + i, i);
    }

    asm volatile("movl %%eax, %%cr3\n" ::"a"(MEMORY_PAGING));
}

static void enable_long_mode()
{
    // open pae...
    LOGK("enable pae...\n");
    asm volatile(
        "movl %cr4, %eax\n"
        "btsl $5, %eax\n"
        "movl %eax, %cr4\n");

    // enable long mode...
    LOGK("enable long mode...\n");
    asm volatile(
        "rdmsr\n"
        "btsl $8, %%eax\n"
        "wrmsr\n" ::"c"(MSR_IA32_EFER));

    // enable paging...
    LOGK("enable paging...\n");
    asm volatile(
        "movl %cr0, %eax\n"
        "btsl $31, %eax\n"
        "movl %eax, %cr0\n");
}

_extern void i386_init(u32 magic, u32 addr)
{
    device::device_init();
    device::none_init();
    device::console_init();
    device::serial_init();

    printk("onix running in protected mode...\n");
    memory_init(magic, addr);
    gdt_init();
    paging_init();
    enable_long_mode();
}