#include <onix/spinlock.h>

void spin_init(spinlock_t *lock)
{
    lock->state = 0;
}

void spin_lock(spinlock_t *lock)
{
    asm volatile(
        ".acquire: \n"
        "lock btsl $0, %0 \n"
        "jnc .finish \n"
        ".spinwait: \n"
        "pause \n"
        "testl $1, %0 \n"
        "jnz .spinwait \n"
        "jmp .acquire \n"
        ".finish: \n"
        : "=m"(lock->state));
}

void spin_unlock(spinlock_t *lock)
{
    asm volatile(
        "movl $0, %0\n"
        : "=m"(lock->state));
}