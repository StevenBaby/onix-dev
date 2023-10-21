#ifndef ONIX_STRING_H
#define ONIX_STRING_H

#include <onix/types.h>

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t size);
char *strcat(char *dest, const char *src);
size_t strlen(const char *str);
size_t strnlen(const char *str, size_t maxlen);
int strcmp(const char *lhs, const char *rhs);
char *strchr(const char *str, int ch);
char *strrchr(const char *str, int ch);

int memcmp(const void *lhs, const void *rhs, size_t size);
void *memset(void *dest, int ch, size_t size);
void *memcpy(void *dest, const void *src, size_t size);
void *memchr(const void *ptr, int ch, size_t size);

#endif