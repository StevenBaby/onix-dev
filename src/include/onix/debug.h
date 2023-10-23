
#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

#define bochs_breakpoint() asm volatile("xchgw %bx, %bx") // bochs magic breakpoint

#endif
