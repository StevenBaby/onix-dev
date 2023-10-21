#include <onix/string.h>

char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == 0)
            return dest;
    }
}

char *strncpy(char *dest, const char *src, size_t size)
{
    char *ptr = dest;
    size_t nr = 0;
    for (; nr < size; nr++)
    {
        *ptr++ = *src;
        if (*src++ == 0)
            return dest;
    }
    dest[size - 1] = 0;
    return dest;
}

char *strcat(char *dest, const char *src)
{
    char *ptr = dest;
    while (*ptr != 0)
    {
        ptr++;
    }
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == 0)
        {
            return dest;
        }
    }
}

size_t strnlen(const char *str, size_t maxlen)
{
    char *ptr = (char *)str;
    while (*ptr != 0 && maxlen--)
    {
        ptr++;
    }
    return ptr - str;
}

size_t strlen(const char *str)
{
    char *ptr = (char *)str;
    while (*ptr != 0)
    {
        ptr++;
    }
    return ptr - str;
}

int strcmp(const char *lhs, const char *rhs)
{
    while (*lhs == *rhs && *lhs != 0 && *rhs != 0)
    {
        lhs++;
        rhs++;
    }
    return *lhs < *rhs ? -1 : *lhs > *rhs;
}

char *strchr(const char *str, int ch)
{
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            return ptr;
        }
        if (*ptr++ == 0)
        {
            return NULL;
        }
    }
}

char *strrchr(const char *str, int ch)
{
    char *last = NULL;
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            last = ptr;
        }
        if (*ptr++ == 0)
        {
            return last;
        }
    }
}

int memcmp(const void *lhs, const void *rhs, size_t size)
{
    char *lptr = (char *)lhs;
    char *rptr = (char *)rhs;
    while ((size > 0) && *lptr == *rptr)
    {
        lptr++;
        rptr++;
        size--;
    }
    if (size == 0)
        return 0;
    return *lptr < *rptr ? -1 : *lptr > *rptr;
}

void *memset(void *dest, int ch, size_t size)
{
    char *ptr = dest;
    while (size--)
    {
        *ptr++ = ch;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t size)
{
    char *ptr = dest;
    while (size--)
    {
        *ptr++ = *((char *)(src++));
    }
    return dest;
}

void *memchr(const void *str, int ch, size_t size)
{
    char *ptr = (char *)str;
    while (size--)
    {
        if (*ptr == ch)
        {
            return (void *)ptr;
        }
        ptr++;
    }
}
