
#include <onix/types.h>
#include <onix/stdlib.h>
#include <onix/string.h>
#include <onix/memory.h>
#include <onix/interrupt.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

u64 memory_max_size = 0;
u64 memory_map_size = 0;
u64 pte_page = 0;

ards_t *ards_max = NULL;
ards_t *ards_base = NULL;
page_t *memory_map = (page_t *)MEMORY_BASE;

// 得到 cr2 寄存器
u64 get_cr2()
{
    asm volatile("movq %cr2, %rax\n");
}

// 刷新虚拟地址 vaddr 的 块表 TLB
void flush_tlb(u64 vaddr)
{
    asm volatile("invlpg (%0)" ::"r"(vaddr)
                 : "memory");
}

// 初始化页表项
static void entry_init(page_entry_t *entry, u32 index)
{
    *(u64 *)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

// 分配一页物理内存
static u64 get_page()
{
    u64 addr = 0;
    if (pte_page)
    {
        pte_page -= PAGE_SIZE;
        addr = pte_page;
    }
    LOGK("get page %#p\n", addr);
    return addr;
}

page_entry_t *get_pte(u64 vaddr, int level, bool create)
{
    if (level == 1)
    {
        page_entry_t *entry = (page_entry_t *)0xFFFFFFFFFFFFF000L;
        assert(entry->present);
        return entry;
    }

    page_entry_t *pde = get_pte(vaddr, level - 1, create);
    int idx = PIDX(vaddr, level - 1);
    page_entry_t *entry = &pde[idx];

    assert(create || (!create && entry->present));

    page_entry_t *pte = (page_entry_t *)(((u64)pde << 9) | (idx << 12));
    if (!entry->present)
    {
        u32 page = get_page();
        entry_init(entry, IDX(page));
        memset(pte, 0, PAGE_SIZE);
    }

    return pte;
}

page_entry_t *get_entry(u64 vaddr, bool create)
{
    page_entry_t *pte = get_pte(vaddr, 4, create);
    int idx = PIDX4(vaddr);
    return &pte[idx];
}

void map_page(u64 vaddr, u64 paddr)
{
    page_entry_t *entry = get_entry(vaddr, true);
    if (entry->present)
        return;

    if (!paddr && vaddr)
        paddr = get_page();

    entry_init(entry, IDX(paddr));
    flush_tlb(vaddr);
}

void map_area(u64 paddr, u64 size)
{
    u32 page_count = div_round_up(size, PAGE_SIZE);
    for (size_t i = 0; i < page_count; i++)
    {
        map_page(paddr + i * PAGE_SIZE, paddr + i * PAGE_SIZE);
    }
    LOGK("MAP memory 0x%p size 0x%X\n", paddr, size);
}

ards_t *memory_area(u64 addr)
{
    ards_t *ptr = ards_table;
    for (size_t i = 0; i < ards_count; i++, ptr++)
    {
        if (ptr->base <= addr && ptr->base + ptr->size > addr)
        {
            return ptr;
        }
    }
    return NULL;
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

    set_interrupt_handler(INTR_PF, page_fault);

    ards_t *ards = ards_table;

    for (size_t i = 0; i < ards_count; i++, ards++)
    {
        LOGK("Memory base 0x%p size 0x%p type %d\n", ards->base, ards->size, ards->type);
        if (ards->type == ARDS_ZONE_VALID)
        {
            memory_max_size = MAX(memory_max_size, ards->base + ards->size);
            ards_max = ards;
        }
        if (ards->base == MEMORY_BASE)
        {
            ards_base = ards;
        }
    }

    pte_page = memory_max_size;

    LOGK("Memory max size 0x%x\n", memory_max_size);
    LOGK("kernel chksum 0x%08x\n", kernel_chksum);
    LOGK("ards base %#x size %#x type %d\n",
         ards_base->base, ards_base->size, ards_base->type);

    memory_map_size = memory_max_size / PAGE_SIZE * sizeof(page_t);
    u64 memory_map_page = div_round_up(memory_map_size, PAGE_SIZE);
    LOGK("Memory map size %#x\n", memory_map_size);
    for (size_t i = 0; i < memory_map_page; i++)
    {
        map_page(MEMORY_BASE + i * PAGE_SIZE, MEMORY_BASE + i * PAGE_SIZE);
    }

    memset(memory_map, 0, memory_map_page * PAGE_SIZE);

    for (size_t i = 0; i < memory_map_page + IDX(MEMORY_BASE); i++)
    {
        memory_map[i].count += 1;
    }

    for (size_t page = pte_page; page < memory_max_size; page += PAGE_SIZE)
    {
        memory_map[IDX(page)].count += 1;
    }

    assert(pte_page > ards_max->base);
    ards_max->size = pte_page - ards_max->base;
    pte_page = 0;
}
