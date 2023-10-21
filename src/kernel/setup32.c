#include <onix/onix.h>
#include <onix/types.h>

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

void setup_long_mode(u32 magic, u32 addr)
{
    check_system_memory();

    console_init();
    puts("Setting up long mode...\n");
}
