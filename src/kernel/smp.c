#include <onix/types.h>
#include <onix/cpu.h>
#include <onix/apic.h>
#include <onix/debug.h>
#include <onix/printk.h>
#include <onix/spinlock.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

spinlock_t lock;

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

    spin_init(&lock);
    apic_send_ap_init();
    apic_send_ap_startup();
}

void smp_ap_init()
{
    spin_lock(&lock);
    LOGK("application processor id %d init...\n", apic_local_id());
    spin_unlock(&lock);
}
