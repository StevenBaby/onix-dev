#ifndef ONIX_PRINTK_H
#define ONIX_PRINTK_H

#include <onix/types.h>

err_t console_write(void *ptr, char *data, size_t size);
err_t serial_write(void *ptr, char *data, size_t size);

#endif