#ifndef ONIX_APIC_H
#define ONIX_APIC_H

#include <onix/types.h>

u32 apic_local_id();

void apic_send_eoi(int vector);
void apic_interrupt_mask(u32 irq, bool enable);

void apic_send_ap_init();
void apic_send_ap_startup();
void apic_send_ipi(int vector, u8 apic_id);

extern bool apic_valid;

#endif
