#ifndef ONIX_ATOMIC_H
#define ONIX_ATOMIC_H

#include <onix/types.h>

typedef struct atomic_t
{
    volatile i64 value;
} atomic_t;

void atomic_add(atomic_t *atomic, i64 value);
void atomic_sub(atomic_t *atomic, i64 value);
void atomic_inc(atomic_t *atomic);
void atomic_dec(atomic_t *atomic);

#endif