#include <onix/types.h>
#include <onix/cpu.h>
#include <onix/apic.h>
#include <onix/debug.h>
#include <onix/printk.h>
#include <onix/interrupt.h>
#include <onix/spinlock.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

spinlock_t lock;
static u32 cpu_count = 1;

#define SMP_IPI_NR 0x90

static void smp_ipi_handler(u64 vector)
{
    LOGK("smp ipi %d\n", vector);
}

void smp_init()
{
    LOGK("smp init...\n");
    cpuid_t id;
    for (int i = 0;; i++)
    {
        cpuid(0xB, i, &id);
        if (((id.ecx >> 8) & 0xff) == 0)
            break;
        LOGK("local apic id type %x width:%#x cpu: %d\n",
             id.ecx >> 8 & 0xff, id.eax & 0x1f, id.ebx & 0xff);
    }
    LOGK("level %#x apic id %#x\n", id.ecx & 0xff, id.edx);

    set_interrupt_handler(SMP_IPI_NR, smp_ipi_handler);

    spin_init(&lock);
    apic_send_ap_init();
    apic_send_ap_startup();

    while (cpu_count < 4)
        ;

    LOGK("cpu init finished\n");

    apic_send_ipi(SMP_IPI_NR, 1);
    apic_send_ipi(SMP_IPI_NR, 1);
    apic_send_ipi(SMP_IPI_NR, 1);
    apic_send_ipi(SMP_IPI_NR, 1);
    apic_send_ipi(SMP_IPI_NR, 1);
    apic_send_ipi(SMP_IPI_NR, 1);
}

extern void local_apic_init();

void smp_ap_init()
{
    local_apic_init();

    set_interrupt_state(true);
    spin_lock(&lock);
    cpu_count += 1;
    spin_unlock(&lock);
    while (true)
    {
        LOGK("application processor id %d init...\n", apic_local_id());
        halt();
    }
}
