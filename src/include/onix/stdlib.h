#ifndef ONIX_STDLIB_H
#define ONIX_STDLIB_H

#include <onix/types.h>

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

#define ALIGN(addr, bytes) (((addr) + (bytes - 1)) & ~(bytes - 1))

#endif