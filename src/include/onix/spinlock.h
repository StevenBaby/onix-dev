#ifndef ONIX_SPINLOCK_H
#define ONIX_SPINLOCK_H

#include <onix/types.h>

typedef struct spinlock_t
{
    volatile u32 state;
} spinlock_t;

void spin_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif