#ifndef ONIX_APIC_H
#define ONIX_APIC_H

#include <onix/types.h>

void apic_send_eoi(int vector);
void apic_interrupt_mask(u32 irq, bool enable);

extern bool apic_valid;

#endif
