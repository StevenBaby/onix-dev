#include <onix/device.h>
#include <onix/io.h>
#include <onix/string.h>

#define CRT_ADDR_REG 0x3D4 // CRT(6845)索引寄存器
#define CRT_DATA_REG 0x3D5 // CRT(6845)数据寄存器

#define CRT_START_ADDR_H 0xC // 显示内存起始位置 - 高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置 - 低位
#define CRT_CURSOR_H 0xE     // 光标位置 - 高位
#define CRT_CURSOR_L 0xF     // 光标位置 - 低位

#define MEM_BASE 0xB8000              // 显卡内存起始位置
#define MEM_SIZE 0x4000               // 显卡内存大小
#define MEM_END (MEM_BASE + MEM_SIZE) // 显卡内存结束位置
#define WIDTH 80                      // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROW_SIZE (WIDTH * 2)          // 每行字节数
#define SCR_SIZE (ROW_SIZE * HEIGHT)  // 屏幕字节数

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

enum ST
{
    ST_NORMAL = 0,
    ST_BOLD = 1,
    ST_BLINK = 5,
    ST_REVERSE = 7,
};

#define STYLE 7
#define BLINK 0x80
#define BOLD 0x0F
#define UNDER 0X0F

enum COLOR
{
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7,
};

#define ERASE 0x0720
#define ARG_NR 16

enum state
{
    STATE_NOR,
    STATE_ESC,
    STATE_QUE,
    STATE_ARG,
    STATE_CSI,
};

namespace arch::device
{
    struct console_t : public device_t
    {
        word_t mem_base; // 内存基地址
        word_t mem_size; // 内存大小
        word_t mem_end;  // 内存结束位置

        word_t screen;   // 当前屏幕位置
        word_t scr_size; // 屏幕内存大小

        word_t pos; // 当前光标位置

        u32 x;        // 光标坐标 x
        u32 y;        // 光标坐标 y
        u32 saved_x;  // 保存的 x
        u32 saved_y;  // 保存的 y
        u32 width;    // 屏幕宽度
        u32 height;   // 屏幕高度
        u32 row_size; // 行内存大小

        u8 state;         // 当前状态
        u32 args[ARG_NR]; // 参数
        u32 argc;         // 参数数量
        u32 ques;         //

        u16 erase; // 清屏字符
        u8 style;  // 当前样式

        console_t()
        {
            this->type = DEVICE_CONSOLE;
            this->mem_base = MEM_BASE;
            this->mem_size = (MEM_SIZE / ROW_SIZE) * ROW_SIZE;
            this->mem_end = this->mem_base + this->mem_size;
            this->width = WIDTH;
            this->height = HEIGHT;
            this->row_size = this->width * 2;
            this->scr_size = this->width * this->height * 2;

            this->erase = ERASE;
            this->style = STYLE;

            this->get_screen();
            this->get_cursor();
        }

        // 写控制台
        int write(char *data, size_t size)
        {

            char ch;
            int nr = 0;
            while (nr++ < size)
            {
                ch = *data++;
                switch (this->state)
                {
                case STATE_NOR:
                    state_normal(ch);
                    break;
                case STATE_ESC:
                    state_esc(ch);
                    break;
                case STATE_QUE:
                    memset(this->args, 0, sizeof(this->args));
                    this->argc = 0;
                    this->state = STATE_ARG;
                    this->ques = (ch == '?');
                    if (this->ques)
                        break;
                case STATE_ARG:
                    if (!state_arg(ch))
                        break;
                case STATE_CSI:
                    state_csi(ch);
                    break;
                default:
                    break;
                }
            }
            set_cursor();
            return nr;
        }

    private:
        // 设置当前显示器开始的位置
        void set_screen()
        {
            io::outb(CRT_ADDR_REG, CRT_START_ADDR_H); // 开始位置高地址
            io::outb(CRT_DATA_REG, ((this->screen - this->mem_base) >> 9) & 0xff);
            io::outb(CRT_ADDR_REG, CRT_START_ADDR_L); // 开始位置低地址
            io::outb(CRT_DATA_REG, ((this->screen - this->mem_base) >> 1) & 0xff);
        }

        // 获取当前显示器开始位置
        void get_screen()
        {

            io::outb(CRT_ADDR_REG, CRT_START_ADDR_H); // 开始位置高地址
            u8 high = io::inb(CRT_DATA_REG);
            io::outb(CRT_ADDR_REG, CRT_START_ADDR_L); // 开始位置低地址
            u8 low = io::inb(CRT_DATA_REG);
            this->screen = this->mem_base + (high << 9 | low << 1);
        }

        // 设置当前显示器光标位置
        void set_cursor()
        {
            io::outb(CRT_ADDR_REG, CRT_CURSOR_H); // 光标高地址
            io::outb(CRT_DATA_REG, ((this->pos - this->mem_base) >> 9) & 0xff);
            io::outb(CRT_ADDR_REG, CRT_CURSOR_L); // 光标低地址
            io::outb(CRT_DATA_REG, ((this->pos - this->mem_base) >> 1) & 0xff);
        }

        // 获取当前光标位置
        void get_cursor()
        {
            io::outb(CRT_ADDR_REG, CRT_CURSOR_H); // 光标高地址
            u8 high = io::inb(CRT_DATA_REG);
            io::outb(CRT_ADDR_REG, CRT_CURSOR_L); // 光标低地址
            u8 low = io::inb(CRT_DATA_REG);
            this->pos = this->mem_base + (high << 9 | low << 1);

            u64 offset = this->pos - this->screen;
            this->x = (offset % this->row_size) >> 1;
            this->y = (offset / this->row_size);
        }

        // 设置光标位置
        void set_xy(u32 x, u32 y)
        {
            if (x > this->width || y >= this->height)
                return;
            this->x = x;
            this->y = y;
            this->pos = this->screen + y * this->row_size + (x << 1);
            // set_cursor();
        }

        // 保存光标位置
        void save_cursor()
        {
            this->saved_x = this->x;
            this->saved_y = this->y;
        }

        // 清除屏幕
        void erase_screen(u16 *start, u32 count)
        {
            int nr = 0;
            while (nr++ < count)
            {
                *start++ = this->erase;
            }
        }

        // 清空控制台
        void clear()
        {
            this->screen = this->mem_base;
            this->pos = this->mem_base;
            this->x = this->y = 0;

            set_cursor();
            set_screen();
            erase_screen((u16 *)this->mem_base, this->mem_size >> 1);
        }

        // 向上滚屏
        void scroll_up()
        {
            if ((this->screen + this->scr_size + this->row_size) >= this->mem_end)
            {
                memcpy((void *)this->mem_base, (void *)this->screen, this->scr_size);
                this->pos -= (this->screen - this->mem_base);
                this->screen = this->mem_base;
            }

            u16 *ptr = (u16 *)(this->screen + this->scr_size);
            erase_screen(ptr, this->width);

            this->screen += this->row_size;
            this->pos += this->row_size;
            set_screen();
        }

        // 向下滚屏
        void scroll_down()
        {
            this->screen -= this->row_size;
            if (this->screen < this->mem_base)
            {
                this->screen = this->mem_base;
            }
            set_screen();
        }

        // 换行 \n
        void lf()
        {
            if (this->y + 1 < this->height)
            {
                this->y++;
                this->pos += this->row_size;
                return;
            }
            scroll_up();
        }

        // 光标回到开始位置
        void cr()
        {
            this->pos -= (this->x << 1);
            this->x = 0;
        }

        // TAB
        void tab()
        {
            int offset = 8 - (this->x & 7);
            this->x += offset;
            this->pos += offset << 1;
            if (this->x >= this->width)
            {
                this->x -= this->width;
                this->pos -= this->row_size;
                lf();
            }
        }

        // 退格
        void bs()
        {
            if (!this->x)
                return;
            this->x--;
            this->pos -= 2;
            *(u16 *)this->pos = this->erase;
        }

        // 删除当前字符
        void del()
        {
            *(u16 *)this->pos = this->erase;
        }

        // 输出字符
        void chr(char ch)
        {
            if (this->x >= this->width)
            {
                this->x -= this->width;
                this->pos -= this->row_size;
                lf();
            }

            *(u16 *)this->pos++ = ch;
            *(u16 *)this->pos++ = this->style;
            this->x++;
        }

        // 正常状态
        void state_normal(char ch)
        {
            switch (ch)
            {
            case NUL:
                break;
            case BEL:
                // start_beep();
                break;
            case BS:
                bs();
                break;
            case HT:
                tab();
                break;
            case LF:
                lf();
                cr();
                break;
            case VT:
            case FF:
                lf();
                break;
            case CR:
                cr();
                break;
            case DEL:
                del();
                break;
            case ESC:
                this->state = STATE_ESC;
                break;
            default:
                chr(ch);
                break;
            }
        }

        // esc 状态
        void state_esc(char ch)
        {
            switch (ch)
            {
            case '[':
                this->state = STATE_QUE;
                break;
            case 'E':
                lf();
                cr();
                break;
            case 'M':
                // go up
                break;
            case 'D':
                lf();
                break;
            case 'Z':
                // response
                break;
            case '7':
                save_cursor();
                break;
            case '8':
                set_xy(this->saved_x, this->saved_y);
                break;
            default:
                break;
            }
        }

        // 参数状态
        bool state_arg(char ch)
        {
            if (this->argc >= ARG_NR)
                return false;
            if (ch == ';')
            {
                this->argc++;
                return false;
            }
            if (ch >= '0' && ch <= '9')
            {
                this->args[this->argc] = this->args[this->argc] * 10 + ch - '0';
                return false;
            }
            this->argc++;
            this->state = STATE_CSI;
            return true;
        }

        // 清屏
        void csi_J()
        {
            int count = 0;
            u64 start = 0;
            switch (this->args[0])
            {
            case 0: // 擦除屏幕中光标后面的部分
                count = (this->screen + this->scr_size - this->pos) >> 1;
                start = this->pos;
                break;
            case 1: // 擦除屏幕中光标前面的部分
                count = (this->pos - this->screen) >> 1;
                start = this->screen;
                break;
            case 2: // 整个屏幕上的字符
                count = this->scr_size >> 1;
                start = this->screen;
                break;
            default:
                return;
            }

            erase_screen((u16 *)start, count);
        }

        // 删除行
        void csi_K()
        {
            int count = 0;
            u64 start = 0;
            switch (this->args[0])
            {
            case 0: // 删除行光标后
                count = this->width - this->x;
                start = this->pos;
                break;
            case 1: // 删除行光标前
                count = this->x;
                start = this->pos - (this->x << 1);
                break;
            case 2: // 删除整行
                count = this->width;
                start = this->pos - (this->x << 1);
                break;
            default:
                return;
            }

            erase_screen((u16 *)start, count);
        }

        // 插入一行
        void insert_line()
        {
            u16 *start = (u16 *)(this->screen + this->y * this->row_size);
            for (size_t i = 2; true; i++)
            {
                void *src = (void *)(this->mem_end - (i * this->row_size));
                if (src < (void *)start)
                    break;

                memcpy((void *)((word_t)src + this->row_size), src, this->row_size);
            }
            erase_screen((u16 *)(this->screen + (this->y) * this->row_size), this->width);
        }

        // 插入多行
        void csi_L()
        {
            int nr = this->args[0];
            if (nr > this->height)
                nr = this->height;
            else if (!nr)
                nr = 1;
            while (nr--)
            {
                insert_line();
            }
        }

        // 删除一行
        void delete_line()
        {
            u16 *start = (u16 *)(this->screen + this->y * this->row_size);
            for (size_t i = 1; true; i++)
            {
                void *src = start + (i * this->row_size);
                if (src >= (void *)this->mem_end)
                    break;

                memcpy((void *)((word_t)src - this->row_size), src, this->row_size);
            }
            erase_screen((u16 *)(this->mem_end - this->row_size), this->width);
        }

        // 删除多行
        void csi_M()
        {
            int nr = this->args[0];
            if (nr > this->height)
                nr = this->height;
            else if (!nr)
                nr = 1;
            while (nr--)
            {
                delete_line();
            }
        }

        // 删除当前字符
        void delete_char()
        {
            u16 *ptr = (u16 *)this->pos;
            u16 i = this->x;
            while (++i < this->width)
            {
                *ptr = *(ptr + 1);
                ptr++;
            }
            *ptr = this->erase;
        }

        // 删除多个字符
        void csi_P()
        {
            int nr = this->args[0];
            if (nr > this->height)
                nr = this->height;
            else if (!nr)
                nr = 1;
            while (nr--)
            {
                delete_char();
            }
        }

        // 插入字符
        void insert_char()
        {
            u16 *ptr = (u16 *)this->pos + (this->width - this->x - 1);
            while (ptr > (u16 *)this->pos)
            {
                *ptr = *(ptr - 1);
                ptr--;
            }
            *(u16 *)this->pos = this->erase;
        }

        // 插入多个字符
        void csi_at()
        {
            int nr = this->args[0];
            if (nr > this->height)
                nr = this->height;
            else if (!nr)
                nr = 1;
            while (nr--)
            {
                insert_char();
            }
        }

        // 修改样式
        void csi_m()
        {
            this->style = 0;
            for (size_t i = 0; i < this->argc; i++)
            {
                if (this->args[i] == ST_NORMAL)
                    this->style = STYLE;

                else if (this->args[i] == ST_BOLD)
                    this->style = BOLD;

                else if (this->args[i] == BLINK)
                    this->style |= BLINK;

                else if (this->args[i] == ST_REVERSE)
                    this->style = (this->style >> 4) | (this->style << 4);

                else if (this->args[i] >= 30 && this->args[i] <= 37)
                    this->style = this->style & 0xF8 | (this->args[i] - 30);

                else if (this->args[i] >= 40 && this->args[i] <= 47)
                    this->style = this->style & 0x8F | ((this->args[i] - 40) << 4);
            }
            this->erase = (this->style << 8) | 0x20;
        }

        // CSI 状态
        void state_csi(char ch)
        {
            this->state = STATE_NOR;
            switch (ch)
            {
            case 'G':
            case '`':
                if (this->args[0])
                    this->args[0]--;
                set_xy(this->args[0], this->y);
                break;
            case 'A': // 光标上移一行或 n 行
                if (!this->args[0])
                    this->args[0]++;
                set_xy(this->x, this->y - this->args[0]);
                break;
            case 'B':
            case 'e': // 光标下移一行或 n 行
                if (!this->args[0])
                    this->args[0]++;
                set_xy(this->x, this->y + this->args[0]);
                break;
            case 'C':
            case 'a': // 光标右移一列或 n 列
                if (!this->args[0])
                    this->args[0]++;
                set_xy(this->x + this->args[0], this->y);
                break;
            case 'D': // 光标左移一列或 n 列
                if (!this->args[0])
                    this->args[0]++;
                set_xy(this->x - this->args[0], this->y);
                break;
            case 'E': // 光标下移一行或 n 行，并回到 0 列
                if (!this->args[0])
                    this->args[0]++;
                set_xy(0, this->y + this->args[0]);
                break;
            case 'F': // 光标上移一行或 n 行，并回到 0 列
                if (!this->args[0])
                    this->args[0]++;
                set_xy(0, this->y - this->args[0]);
                break;
            case 'd': // 设置行号
                if (this->args[0])
                    this->args[0]--;
                set_xy(this->x, this->args[0]);
                break;
            case 'H': // 设置行号和列号
            case 'f':
                if (this->args[0])
                    this->args[0]--;
                if (this->args[1])
                    this->args[1]--;
                set_xy(this->args[1], this->args[0]);
                break;
            case 'J': // 清屏
                csi_J();
                break;
            case 'K': // 行删除
                csi_K();
                break;
            case 'L': // 插入行
                csi_L();
                break;
            case 'M': // 删除行
                csi_M();
                break;
            case 'P': // 删除字符
                csi_P();
                break;
            case '@': // 插入字符
                csi_at();
                break;
            case 'm': // 修改样式
                csi_m();
                break;
            case 'r': // 设置起始行号和终止行号
                break;
            case 's':
                save_cursor();
            case 'u':
                set_xy(this->saved_x, this->saved_y);
            default:
                break;
            }
        }
    };
}

static device::console_t console;

void device::console_init()
{
    auto ptr = new (&console) console_t();
    device::install(ptr);
}