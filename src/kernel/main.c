
#include <onix/types.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/interrupt.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void console_init();
extern void serial_init();
extern void memory_init();
extern void zone_init();
extern void interrupt_init();
extern void pic_init();
extern void clock_init();
extern void keyboard_init();
extern void cpu_init();

void kernel_init()
{
    console_init();
    serial_init();
    interrupt_init();
    memory_init();
    zone_init();
    cpu_init();
    pic_init();
    clock_init();
    keyboard_init();

    set_interrupt_state(true);
}
