
#include <onix/types.h>
#include <onix/io.h>
#include <onix/string.h>
#include <onix/printk.h>

char message[] = "Onix 64 bit is running...\n";

extern void console_init();
extern void serial_init();

void kernel_init()
{
    console_init();
    serial_init();

    printk("This is test of printf %d %c %s %p\n",
           20231019, 'A', "hello world", &message);
}
