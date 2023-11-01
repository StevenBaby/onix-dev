#include <onix/assert.h>

void *operator new(size_t n)
{
    panic("operator new");
    return NULL; // todo
}

void operator delete(void *, unsigned long)
{
    panic("operator delete");
}

void operator delete(void *)
{
    panic("operator delete");
}

namespace std
{
    void __throw_bad_alloc()
    {
        panic("throw bad alloc");
    }

    void __throw_bad_array_new_length()
    {
        panic("throw bad alloc");
    }

}