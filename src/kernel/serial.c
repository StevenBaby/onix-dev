#include <onix/io.h>
#include <onix/errno.h>

#define COM1_IOBASE 0x3F8 // 串口 1 基地址
#define COM2_IOBASE 0x2F8 // 串口 2 基地址

#define COM_DATA 0          // 数据寄存器
#define COM_INTR_ENABLE 1   // 中断允许
#define COM_BAUD_LSB 0      // 波特率低字节
#define COM_BAUD_MSB 1      // 波特率高字节
#define COM_INTR_IDENTIFY 2 // 中断识别
#define COM_LINE_CONTROL 3  // 线控制
#define COM_MODEM_CONTROL 4 // 调制解调器控制
#define COM_LINE_STATUS 5   // 线状态
#define COM_MODEM_STATUS 6  // 调制解调器状态

// 线状态
#define LSR_DR 0x1
#define LSR_OE 0x2
#define LSR_PE 0x4
#define LSR_FE 0x8
#define LSR_BI 0x10
#define LSR_THRE 0x20
#define LSR_TEMT 0x40
#define LSR_IE 0x80

#define IRQ_SERIAL_2 3 // 串口 2
#define IRQ_SERIAL_1 4 // 串口 1

typedef struct serial_t
{
    u16 iobase; // 端口号基地址
} serial_t;

static serial_t serials[2];

// 写串口
err_t serial_write(void *ptr, char *data, size_t size)
{
    serial_t *serial = (serial_t *)ptr;
    if (serial == NULL)
        serial = serials;

    int nr = 0;
    while (nr < size)
    {
        u8 state = inb(serial->iobase + COM_LINE_STATUS);
        if (state & LSR_THRE) // 如果串口可写
        {
            outb(serial->iobase, data[nr++]);
            continue;
        }
    }
    return nr;
}

// 初始化串口
void serial_init()
{
    for (size_t i = 0; i < 2; i++)
    {
        serial_t *serial = &serials[i];

        u16 irq;
        if (!i)
        {
            irq = IRQ_SERIAL_1;
            serial->iobase = COM1_IOBASE;
        }
        else
        {
            irq = IRQ_SERIAL_2;
            serial->iobase = COM2_IOBASE;
        }

        // 禁用中断
        outb(serial->iobase + COM_INTR_ENABLE, 0);

        // 激活 DLAB
        outb(serial->iobase + COM_LINE_CONTROL, 0x80);

        // 设置波特率因子 0x0030
        // 波特率为 115200 / 0x30 = 115200 / 48 = 2400
        outb(serial->iobase + COM_BAUD_LSB, 0x30);
        outb(serial->iobase + COM_BAUD_MSB, 0x00);

        // 复位 DLAB 位，数据位为 8 位
        outb(serial->iobase + COM_LINE_CONTROL, 0x03);

        // 启用 FIFO, 清空 FIFO, 14 字节触发电平
        outb(serial->iobase + COM_INTR_IDENTIFY, 0xC7);

        // 设置回环模式，测试串口芯片
        outb(serial->iobase + COM_MODEM_CONTROL, 0b11011);

        // 发送字节
        outb(serial->iobase, 0xAE);

        // 收到的内容与发送的不一致，则串口不可用
        if (inb(serial->iobase) != 0xAE)
        {
            continue;
        }

        // 设置回原来的模式
        outb(serial->iobase + COM_MODEM_CONTROL, 0b1011);
    }
}