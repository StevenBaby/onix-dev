#ifndef ONIX_PRINTK_H
#define ONIX_PRINTK_H

#include <onix/types.h>
#include <onix/stdarg.h>

err_t console_write(void *ptr, char *data, size_t size);
err_t serial_write(void *ptr, char *data, size_t size);

err_t vsprintf(char *buf, const char *fmt, va_list args);
err_t sprintf(char *buf, const char *fmt, ...);
err_t printk(const char *fmt, ...);

#endif