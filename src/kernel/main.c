
#include <onix/types.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/printk.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

char message;

extern void console_init();
extern void serial_init();

void kernel_init()
{
    console_init();
    serial_init();

    LOGK("This is test of printf %d %c %s %p\n",
           20231019, 'A', "hello world", &message);
}
