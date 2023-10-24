
#include <onix/types.h>
#include <onix/stdlib.h>
#include <onix/memory.h>
#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

u64 memory_max_size = 0;

// 得到 cr2 寄存器
u64 get_cr2()
{
    asm volatile("movq %cr2, %rax\n");
}

void page_fault(
    u64 _rdi, u64 _rsi, u64 _rdx, u64 _rcx, u64 _r8, u64 _r9,
    u64 gs, u64 fs, u64 es, u64 ds,
    u64 r15, u64 r14, u64 r13, u64 r12, u64 r11, u64 r10, u64 r9, u64 r8,
    u64 rdi, u64 rsi, u64 rbx, u64 rdx, u64 rcx, u64 rax, u64 rbp,
    u64 vector, u64 error, u64 rip, u64 cs, u64 eflags)
{
    u64 vaddr = get_cr2();
    LOGK("fault address 0x%p rip 0x%p\n", vaddr, rip);
    panic("page fault");
}

void memory_init()
{
    LOGK("memory init...\n");
    ards_t *ards = ards_table;
    for (size_t i = 0; i < ards_count; i++, ards++)
    {
        LOGK("Memory base 0x%p size 0x%p type %d\n", ards->base, ards->size, ards->type);
        if (ards->type == ARDS_ZONE_VALID)
        {
            memory_max_size = MAX(memory_max_size, ards->base + ards->size);
        }
    }

    LOGK("Memory max size 0x%p\n", memory_max_size);
    LOGK("kernel chksum 0x%08x\n", kernel_chksum);

    set_interrupt_handler(INTR_PF, page_fault);

    char *ptr = (char *)0x1000000000;
    *ptr = 'a';
}
