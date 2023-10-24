#ifndef ONIX_ACPI_H
#define ONIX_ACPI_H

#include <onix/types.h>

#define MAX_CPU_COUNT 256

extern u32 cpu_apic_ids[MAX_CPU_COUNT];
extern u32 cpu_count;

#endif
