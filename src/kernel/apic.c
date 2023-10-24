#include <onix/cpu.h>
#include <onix/memory.h>
#include <onix/io.h>
#include <onix/mio.h>
#include <onix/pic.h>
#include <onix/apic.h>
#include <onix/clock.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define MSR_APIC_BASE 0x1B

#define CPUID_EDX_ACPI (1 << 9)
#define CPUID_ECX_x2APIC (1 << 21)

#define APIC_ID (0x20)
#define APIC_VERSION (0x30)
#define APIC_TPR (0x80)  // Task Priority Register
#define APIC_APR (0x90)  // Arbitration Priority Register
#define APIC_PPR (0xA0)  // Processor Priority Register
#define APIC_EOI (0xB0)  // EOI Register
#define APIC_RRD (0xC0)  // Remote Read Register
#define APIC_LDR (0xD0)  // Logical Destination Register
#define APIC_DFR (0xE0)  // Destination Format Register
#define APIC_SIVR (0xF0) // Spurious Interrupt Vector Register

#define APIC_ICR_LOW (0x300)
#define APIC_ICR_HIGH (0x310)

#define APIC_LVT_CMCI (0x2F0)
#define APIC_LVT_TIMER (0x320)
#define APIC_LVT_THERMAL (0x330)
#define APIC_LVT_PC (0x340)
#define APIC_LVT_LINT0 (0x350)
#define APIC_LVT_LINT1 (0x360)
#define APIC_LVT_ERROR (0x370)

#define APIC_TIMER_INIT (0x380)
#define APIC_TIMER_CUR (0x390)
#define APIC_TIMER_DIV (0x3E0)

#define IOAPIC_DATA (0x10)
#define IOAPIC_EOI (0x40)

#define IOAPIC_ID (0x00)
#define IOAPIC_VERSION (0x01)
#define IOREDTBL (0x10)

// Figure 10-7. Local APIC Version Register
#define APIC_VERSION_VER (0)
#define APIC_VERSION_LVT (16)
#define APIC_VERSION_EOI (24)

// Figure 10-5. IA32_APIC_BASE MSR (APIC_BASE_MSR in P6 Family)
#define APIC_BASE_BSP (1 << 8)
#define APIC_BASE_EXT (1 << 10)
#define APIC_BASE_EN (1 << 11)

// Figure 10-12. Interrupt Command Register (ICR) page 3177
#define APIC_ICR_DELIVERY_FIXED (0b000 << 8)
#define APIC_ICR_DELIVERY_LOW (0b001 << 8)
#define APIC_ICR_DELIVERY_SMI (0b010 << 8)
#define APIC_ICR_DELIVERY_NMI (0b100 << 8)
#define APIC_ICR_DELIVERY_INIT (0b101 << 8)
#define APIC_ICR_DELIVERY_START (0b110 << 8)

#define APIC_ICR_DST_PHY (0 << 11)
#define APIC_ICR_DST_LOG (1 << 11)
#define APIC_ICR_DEL_IDLE (0 << 12)
#define APIC_ICR_DEL_PEND (1 << 12)
#define APIC_ICR_LEVEL_DEASSERT (0 << 14)
#define APIC_ICR_LEVEL_ASSERT (1 << 14)
#define APIC_ICR_TRIGGER_EDGE (0 << 15)
#define APIC_ICR_TRIGGER_LEVEL (1 << 15)

#define APIC_ICR_DST_NO_SHORTHAND (0b00 << 18)
#define APIC_ICR_DST_SELF (0b01 << 18)
#define APIC_ICR_DST_ALL_WITH_SELF (0b10 << 18)
#define APIC_ICR_DST_ALL_NO_SELF (0b11 << 18)

// Figure 10-23. Spurious-Interrupt Vector Register (SVR)
#define APIC_SIVR_EN (1 << 8)
#define APIC_SIVR_FOCUS (1 << 9)
#define APIC_SIVR_EOI (1 << 12)

#define APIC_LVT_MASKED (1 << 16)
#define APIC_LVT_DELIVERY_FIXED (0b000 << 8)
#define APIC_LVT_DELIVERY_LOW (0b001 << 8)
#define APIC_LVT_DELIVERY_SMI (0b010 << 8)
#define APIC_LVT_DELIVERY_NMI (0b100 << 8)
#define APIC_LVT_DELIVERY_INIT (0b101 << 8)
#define APIC_LVT_DELIVERY_EXTINIT (0b111 << 8)
#define APIC_LVT_TRIGGER_EDGE (0 << 15)
#define APIC_LVT_TRIGGER_LEVEL (1 << 15)
#define APIC_LVT_TIMER_ONESHOT (0 << 17)
#define APIC_LVT_TIMER_PERIODIC (1 << 17)

#define APIC_TIMER_DIV1 (0b1011)
#define APIC_TIMER_DIV2 (0b0000)
#define APIC_TIMER_DIV4 (0b0001)
#define APIC_TIMER_DIV8 (0b0010)
#define APIC_TIMER_DIV16 (0b0011)
#define APIC_TIMER_DIV32 (0b1000)
#define APIC_TIMER_DIV64 (0b1001)
#define APIC_TIMER_DIV128 (0b1010)

bool apic_valid = false;
static u64 local_apic_addr = 0xFEE00000UL;
static u64 io_apic_addr = 0xFEC00000UL;

static void get_msr(u32 msr, u32 *eax, u32 *edx)
{
    asm volatile("rdmsr\n" : "=a"(*eax), "=d"(*edx) : "c"(msr));
}

static void set_msr(u32 msr, u32 eax, u32 edx)
{
    asm volatile("wrmsr\n" : : "a"(eax), "d"(edx), "c"(msr));
}

static u32 local_apic_inl(u32 offset)
{
    return minl(local_apic_addr + offset);
}

static void local_apic_outl(u32 addr, u32 data)
{
    moutl(local_apic_addr + addr, data);
}

static u32 io_apic_inl(u8 idx)
{
    moutl(io_apic_addr, idx);
    return minl(io_apic_addr + IOAPIC_DATA);
}

static void io_apic_outl(u8 idx, u32 data)
{
    moutl(io_apic_addr, idx);
    moutl(io_apic_addr + IOAPIC_DATA, data);
}

static u64 io_apic_rte_inq(u8 idx)
{
    u64 data = io_apic_inl(IOREDTBL + idx * 2 + 1);
    data <<= 32;
    data |= io_apic_inl(IOREDTBL + idx * 2);
    return data;
}

static void io_apic_rte_outq(u8 idx, u64 data)
{
    io_apic_outl(IOREDTBL + idx * 2, (u32)data);
    io_apic_outl(IOREDTBL + idx * 2 + 1, (u32)(data >> 32));
}

u32 apic_local_id()
{
    if (!apic_valid)
        return 0;
    return local_apic_inl(APIC_ID) >> 24;
}

void apic_send_eoi(int vector)
{
    local_apic_outl(APIC_EOI, 0);
}

void apic_interrupt_mask(u32 irq, bool enable)
{
    u64 mask = io_apic_rte_inq(irq);
    if (enable)
        mask &= ~APIC_LVT_MASKED;
    else
        mask |= APIC_LVT_MASKED;
    io_apic_rte_outq(irq, mask);
}

void apic_send_ap_init()
{
    u32 data = APIC_ICR_DST_ALL_NO_SELF;
    data |= APIC_ICR_LEVEL_ASSERT;
    data |= APIC_ICR_DELIVERY_INIT;
    local_apic_outl(APIC_ICR_LOW, data);
    while (local_apic_inl(APIC_ICR_LOW) & APIC_ICR_DEL_PEND)
        ;
}

extern void startsmp();

void apic_send_ap_startup()
{
    u64 data = (u64)startsmp;
    assert((data & 0xfff) == 0);
    data >>= 12;

    data |= APIC_ICR_DST_ALL_NO_SELF;
    data |= APIC_ICR_LEVEL_ASSERT;
    data |= APIC_ICR_DELIVERY_START;
    local_apic_outl(APIC_ICR_LOW, data);
    while (local_apic_inl(APIC_ICR_LOW) & APIC_ICR_DEL_PEND)
        ;
}

void apic_send_ipi(int vector, u8 apic_id)
{
    u32 data = (u32)apic_id << 24;
    local_apic_outl(APIC_ICR_HIGH, data);

    data = vector | APIC_ICR_DELIVERY_FIXED;
    local_apic_outl(APIC_ICR_LOW, data);
}

void local_apic_timer_init()
{
    local_apic_outl(APIC_TIMER_DIV, APIC_TIMER_DIV16);
    local_apic_outl(APIC_TIMER_INIT, 0xFFFFFFFF);
    clock_sleep(10);
    local_apic_outl(APIC_LVT_TIMER, APIC_LVT_MASKED);

    u32 ticks = 0xFFFFFFFF - local_apic_inl(APIC_TIMER_CUR);
    local_apic_outl(APIC_TIMER_DIV, APIC_TIMER_DIV16);
    local_apic_outl(APIC_TIMER_INIT, ticks);
    local_apic_outl(
        APIC_LVT_TIMER,
        (IRQ_APIC_TIMER + IRQ_MASTER_NR) | APIC_LVT_TIMER_PERIODIC);
}

void local_apic_init()
{
    u32 eax;
    u32 edx;
    u32 data;

    data = local_apic_inl(APIC_VERSION);
    LOGK("apic version  %#x\n", data);
    LOGK("apic lvt %#x\n", (data >> 16) & 0xff);
    LOGK("apic support eoi %#x\n", (data >> 24) & 0x1);

    data = local_apic_inl(APIC_ID);
    LOGK("apic id %#x\n", data >> 24);

    get_msr(MSR_APIC_BASE, &eax, &edx);
    set_msr(MSR_APIC_BASE, eax | APIC_BASE_EN, edx);
    get_msr(MSR_APIC_BASE, &eax, &edx);
    LOGK("apic base %#x\n", (u64)edx << 32 | eax);
    assert((eax & ~0xfff) == local_apic_addr);

    LOGK("SIVR %#x\n", local_apic_inl(APIC_SIVR));
    data = local_apic_inl(APIC_SIVR);
    local_apic_outl(APIC_SIVR, (data & ~0xff) | APIC_SIVR_EN | 0xFF);
    LOGK("SIVR %#x\n", local_apic_inl(APIC_SIVR));

    local_apic_outl(APIC_LVT_TIMER, APIC_LVT_MASKED);
    local_apic_outl(APIC_LVT_THERMAL, APIC_LVT_MASKED);
    local_apic_outl(APIC_LVT_PC, APIC_LVT_MASKED);
    local_apic_outl(APIC_LVT_LINT0, APIC_LVT_MASKED);
    local_apic_outl(APIC_LVT_LINT1, APIC_LVT_MASKED);
    local_apic_outl(APIC_LVT_ERROR, APIC_LVT_MASKED);

    LOGK("apic timer %#x\n", local_apic_inl(APIC_LVT_TIMER));
    LOGK("apic thermal %#x\n", local_apic_inl(APIC_LVT_THERMAL));
    LOGK("apic pc %#x\n", local_apic_inl(APIC_LVT_PC));
    LOGK("apic lint0 %#x\n", local_apic_inl(APIC_LVT_LINT0));
    LOGK("apic lint1 %#x\n", local_apic_inl(APIC_LVT_LINT1));
    LOGK("apic error %#x\n", local_apic_inl(APIC_LVT_ERROR));
}

void io_apic_init()
{
    u32 data;

    data = io_apic_inl(IOAPIC_ID);
    LOGK("io apic id %#x\n", data >> 24);

    data = io_apic_inl(IOAPIC_VERSION);
    u32 rte_count = ((data >> 16) & 0xff) + 1;
    LOGK("io apic version %#x\n", data & 0xff);
    LOGK("io apic rte count %d\n", rte_count);

    for (int i = rte_count - 1; i >= 0; i--)
    {
        u32 vector = IRQ_MASTER_NR + i;
        io_apic_rte_outq(i, APIC_LVT_MASKED | vector);
        LOGK("io apic rte %d %#x\n", i, io_apic_rte_inq(i));
    }
}

void apic_init()
{
    cpuid_t id;
    cpuid(1, 0, &id);

    if (!(id.edx & CPUID_EDX_ACPI))
    {
        LOGK("CPU does not support apic...\n");
        return;
    }

    LOGK("apic init...\n");

    // disable pic
    outb(PIC_M_DATA, 0b11111111); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断

    map_area(local_apic_addr, PAGE_SIZE);
    map_area(io_apic_addr, PAGE_SIZE);

    local_apic_init();
    io_apic_init();

    apic_valid = true;
}