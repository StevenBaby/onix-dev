#include <onix/atomic.h>

void atomic_add(atomic_t *atomic, i64 value)
{
    asm volatile(
        "lock addq %1, %0 \n"
        : "=m"(atomic->value)
        : "r"(value));
}

void atomic_sub(atomic_t *atomic, i64 value)
{
    asm volatile(
        "lock subq %1, %0 \n"
        : "=m"(atomic->value)
        : "r"(value));
}

void atomic_inc(atomic_t *atomic)
{
    asm volatile(
        "lock incq %0 \n"
        : "=m"(atomic->value));
}

void atomic_dec(atomic_t *atomic)
{
     asm volatile(
        "lock decq %0 \n"
        : "=m"(atomic->value));
}
