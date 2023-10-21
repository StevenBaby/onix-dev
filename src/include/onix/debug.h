
#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

#define bochs_breakpoint() asm volatile("xchgw %bx, %bx") // bochs magic breakpoint

void debugk(char *file, int line, const char *fmt, ...);

#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

// #define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#endif
