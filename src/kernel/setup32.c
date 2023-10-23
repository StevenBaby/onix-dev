#include <onix/onix.h>
#include <onix/types.h>
#include <onix/debug.h>
#include <onix/stdlib.h>
#include <onix/multiboot2.h>
#include <onix/descriptor.h>
#include <onix/memory.h>

#define MSR_IA32_EFER 0xC0000080

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

static _omit_frame void code32()
{
    asm volatile(".code32\n");
}

static u32 crc32(void *data, int len)
{
    u32 crc = -1;
    u8 *ptr = (u8 *)data;
    for (int i = 0; i < len; i++)
    {
        crc ^= ptr[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}

// in byte
static u8 inb(u16 port)
{
    u8 ret;
    asm volatile(
        "inb %%dx, %%al\n"
        : "=a"(ret)
        : "d"(port)

    );
    return ret;
}

// out byte
static void outb(u16 port, u8 data)
{
    asm volatile(
        "outb %%al, %%dx\n"
        : : "d"(port), "a"(data));
}

static void *memset(void *dest, int ch, size_t size)
{
    char *ptr = dest;
    while (size--)
    {
        *ptr++ = ch;
    }
    return dest;
}

static void *memcpy(void *dest, const void *src, size_t size)
{
    char *ptr = dest;
    while (size--)
    {
        *ptr++ = *((char *)(src++));
    }
    return dest;
}

#define CRT_ADDR_REG 0x3D4 // CRT(6845)索引寄存器
#define CRT_DATA_REG 0x3D5 // CRT(6845)数据寄存器

#define CRT_START_ADDR_H 0xC // 显示内存起始位置 - 高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置 - 低位
#define CRT_CURSOR_H 0xE     // 光标位置 - 高位
#define CRT_CURSOR_L 0xF     // 光标位置 - 低位

#define MEM_BASE 0xB8000              // 显卡内存起始位置
#define MEM_SIZE 0x3F40               // ((0x4000 / ROW_SIZE) * ROW_SIZE)
#define MEM_END (MEM_BASE + MEM_SIZE) //
#define WIDTH 80                      // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROWSIZE (WIDTH * 2)           // 行大小
#define SCRSIZE (ROWSIZE * HEIGHT)    // 屏幕大小

#define STYLE 7
#define ERASE 0x0720

#define NUL 0x00
#define ENQ 0x05
#define ESC 0x1B // ESC
#define BEL 0x07 // \a
#define BS 0x08  // \b
#define HT 0x09  // \t
#define LF 0x0A  // \n
#define VT 0x0B  // \v
#define FF 0x0C  // \f
#define CR 0x0D  // \r
#define DEL 0x7F

static u32 screen = 0;
static u32 pos = 0;
static u32 x = 0;
static u32 y = 0;

// 设置当前显示器开始的位置
static _inline void set_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H); // 开始位置高地址
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xff);
    outb(CRT_ADDR_REG, CRT_START_ADDR_L); // 开始位置低地址
    outb(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xff);
}

// 获取当前显示器开始位置
static void get_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H); // 开始位置高地址
    u8 high = inb(CRT_DATA_REG);
    outb(CRT_ADDR_REG, CRT_START_ADDR_L); // 开始位置低地址
    u8 low = inb(CRT_DATA_REG);
    screen = MEM_BASE + (high << 9 | low << 1);
}

// 设置当前显示器光标位置
static _inline void set_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H); // 光标高地址
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0xff);
    outb(CRT_ADDR_REG, CRT_CURSOR_L); // 光标低地址
    outb(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0xff);
}

// 获取当前光标位置
static void get_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H); // 光标高地址
    u8 high = inb(CRT_DATA_REG);
    outb(CRT_ADDR_REG, CRT_CURSOR_L); // 光标低地址
    u8 low = inb(CRT_DATA_REG);
    pos = MEM_BASE + (high << 9 | low << 1);
    u32 offset = pos - screen;
    x = (offset % ROWSIZE) >> 1;
    y = (offset / ROWSIZE);
}

// 清除屏幕
static _inline void erase_screen(u16 *start, u32 count)
{
    int nr = 0;
    while (nr++ < count)
    {
        *start++ = ERASE;
    }
}

static void scroll_up()
{
    if ((screen + SCRSIZE + ROWSIZE) >= MEM_END)
    {
        memcpy((void *)MEM_BASE, (void *)screen, SCRSIZE);
        pos -= (screen - MEM_BASE);
        screen = MEM_BASE;
    }

    u16 *ptr = (u16 *)(screen + SCRSIZE);
    erase_screen(ptr, WIDTH);

    screen += ROWSIZE;
    pos += ROWSIZE;
    set_screen();
}

static _inline void lf()
{
    if (y + 1 < HEIGHT)
    {
        y++;
        pos += ROWSIZE;
        return;
    }
    scroll_up();
}

// 光标回到开始位置
static _inline void cr()
{
    pos -= (x << 1);
    x = 0;
}

// 输出字符
static _inline void chr(char ch)
{
    if (x >= WIDTH)
    {
        x -= WIDTH;
        pos -= ROWSIZE;
        lf();
    }

    *(char *)pos++ = ch;
    *(char *)pos++ = STYLE;
    x++;
}

static void puts(char *data)
{
    while (*data)
    {
        char ch = *data;
        switch (ch)
        {
        case NUL:
            break;
        case LF:
            lf();
            cr();
            break;
        default:
            chr(ch);
            break;
        }
        data++;
    }
    set_cursor();
}

static void console_init()
{
    get_screen();
    get_cursor();
    puts("Console init in protected mode...\n");
}

extern u8 _end;
extern u8 _start;

u32 kernel_chksum;
u32 kernel_size;

static void check_system_memory()
{
    u32 size = &_end - &_start;
    kernel_chksum = crc32((void *)&_start, size);
    kernel_size = size;
}

#define GDT_SIZE 8

static _aligned8 descriptor_t gdt[GDT_SIZE]; // global descriotpr table
static _aligned8 pointer_t gdt_ptr;          // gdt pointer

static void gdt_init()
{
    puts("GDT init...\n");

    memset(gdt, 0, sizeof(gdt));

    descriptor_t *desc;
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

    asm volatile("lgdt gdt_ptr\n");
}

static _inline void entry_init(page_entry_t *entry, u32 index)
{
    *(u64 *)entry = PAGE(index) | 7;
}

static void paging_init()
{
    puts("paging init...\n");

    memset((void *)MEMORY_PAGING, 0, PAGE_SIZE * 4);
    page_entry_t *entry;

    entry = (page_entry_t *)MEMORY_PAGING;
    entry_init(entry, IDX(MEMORY_PAGING + PAGE_SIZE));

    entry = (page_entry_t *)(MEMORY_PAGING + PAGE_SIZE);
    entry_init(entry, IDX(MEMORY_PAGING + PAGE_SIZE * 2));

    entry = (page_entry_t *)(MEMORY_PAGING + PAGE_SIZE * 2);
    entry_init(entry, IDX(MEMORY_PAGING + PAGE_SIZE * 3));

    entry = (page_entry_t *)(MEMORY_PAGING + PAGE_SIZE * 3);
    for (size_t i = 0; i < 512; i++, entry++)
    {
        entry_init(entry, i);
    }

    asm volatile("movl %%eax, %%cr3\n" ::"a"(MEMORY_PAGING));
}

static void enable_long_mode()
{
    // open pae...
    asm volatile(
        "movl %cr4, %eax\n"
        "btsl $5, %eax\n"
        "movl %eax, %cr4\n");

    // enable long mode...
    asm volatile(
        "rdmsr\n"
        "btsl $8, %%eax\n"
        "wrmsr\n" ::"c"(MSR_IA32_EFER));

    // enable paging...
    asm volatile(
        "movl %cr0, %eax\n"
        "btsl $31, %eax\n"
        "movl %eax, %cr0\n");
}

ards_t _aligned8 ards_table[ARDS_TABLE_LEN];
u32 ards_count = 0;

static void ards_init(u32 addr)
{
    puts("ards init...\n");
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

void setup_long_mode(u32 magic, u32 addr)
{
    check_system_memory();

    console_init();
    puts("Setting up long mode...\n");

    switch (magic)
    {
    case ONIX_MAGIC:
        ards_init(addr);
        break;
    case MULTIBOOT2_MAGIC:
        multiboot2_ards_init(addr);
        break;
    default:
        puts("Magic unknown, init memory failure...\n");
        while (true)
            ;
        break;
    }

    gdt_init();
    paging_init();
    enable_long_mode();
}
