#include <onix/io.h>
#include <onix/interrupt.h>
#include <onix/pic.h>
#include <onix/apic.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000 / HZ)

#define SPEAKER_REG 0x61
#define BEEP_HZ 440
#define BEEP_COUNTER (OSCILLATOR / BEEP_HZ)
#define BEEP_MS 100

u64 volatile jiffies = 0;
u64 jiffy = JIFFY;

void clock_handler(int vector)
{
    assert(vector == 0x20 || vector == 0x22);

    jiffies++;
    // DEBUGK("clock jiffies %d ...\n", jiffies);
}

void pit_init()
{
    // 配置计数器 0 时钟
    outb(PIT_CTRL_REG, 0b00110100);
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);

    // 配置计数器 2 蜂鸣器
    outb(PIT_CTRL_REG, 0b10110110);
    outb(PIT_CHAN2_REG, (u8)BEEP_COUNTER);
    outb(PIT_CHAN2_REG, (u8)(BEEP_COUNTER >> 8));
}

void clock_init()
{
    pit_init();
    int irq = IRQ_CLOCK;
    if (apic_valid)
        irq = IRQ_CASCADE;

    set_interrupt_handler(irq  + IRQ_MASTER_NR, clock_handler);
    set_interrupt_mask(irq, true);
}
