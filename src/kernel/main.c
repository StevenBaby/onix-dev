
#include <onix/types.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/printk.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void console_init();
extern void serial_init();
extern void memory_init();
extern void interrupt_init();

void kernel_init()
{
    console_init();
    serial_init();
    interrupt_init();
    memory_init();
}
