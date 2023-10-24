#include <onix/types.h>
#include <onix/string.h>
#include <onix/acpi.h>
#include <onix/assert.h>
#include <onix/debug.h>
#include <onix/memory.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

typedef struct rsdptr_t
{
    u8 signature[8];
    u8 checksum;
    u8 oem[6];
    u8 revision;
    u32 *rsdt_addr;
} _packed rsdptr_t;

typedef struct acpi_header_t
{
    u32 signature;
    u32 length;
    u8 revision;
    u8 checksum;
    u8 oem[6];
    u8 oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
} _packed acpi_header_t;

typedef struct acpi_fadt_t
{
    acpi_header_t header;
    u32 firmware_control;
    u32 dsdt;
    u8 reserved;
    u8 preferred;
    u16 sci_interrupt;
    u32 smi_command_port;
    u8 acpi_enable;
    u8 acpi_disable;
    // ...
} _packed acpi_fadt_t;

typedef struct acpi_madt_t
{
    acpi_header_t header;
    u32 local_apic_addr;
    u32 flags;
} _packed acpi_madt_t;

typedef struct apic_header_t
{
    u8 type;
    u8 length;
} _packed apic_header_t;

#define APIC_TYPE_LOCAL 0
#define APIC_TYPE_IO 1
#define APIC_TYPE_OVERRIDE 2

typedef struct apic_local_t
{
    apic_header_t header;
    u8 acpi_processor_id;
    u8 acpi_id;
    u32 flags;
} _packed apic_local_t;

typedef struct apic_io_t
{
    apic_header_t header;
    u8 io_apic_id;
    u8 RESERVED;
    u32 io_apic_addr;
    u32 global_base;
} _packed apic_io_t;

typedef struct apic_override_t
{
    apic_header_t header;
    u8 bus;
    u8 source;
    u32 interrupt;
    u16 flags;
} _packed apic_override_t;

static acpi_madt_t *madt;
static char sig[] = "RSD PTR ";

u32 cpu_apic_ids[MAX_CPU_COUNT];
u32 cpu_count = 0;

extern u64 local_apic_addr;
extern u64 io_apic_addr;

rsdptr_t *acpi_check_rsdptr(u32 *ptr)
{
    rsdptr_t *rsdp = (rsdptr_t *)ptr;
    u8 *bptr;
    u8 check = 0;

    if (memcmp(sig, rsdp, 8) == 0)
    {
        // check checksum rsdpd
        bptr = (u8 *)ptr;
        for (int i = 0; i < sizeof(rsdptr_t); i++)
        {
            check += *bptr;
            bptr++;
        }

        // found valid rsdpd
        if (check == 0)
        {
            LOGK("find valid rsdp oem:%s revision:%#x\n", rsdp->oem, rsdp->revision);
            return rsdp;
        }
    }
    return NULL;
}

rsdptr_t *acpi_find_rsdptr()
{
    u32 *addr;
    rsdptr_t *rsdp;

    // search below the 1mb mark for RSDP signature
    for (addr = (u32 *)0x000E0000; (u64)addr < 0x00100000; addr += 0x10 / sizeof(addr))
    {
        rsdp = acpi_check_rsdptr(addr);
        if (rsdp != NULL)
            return rsdp;
    }

    // at address 0x40:0x0E is the RM segment of the ebda
    u64 ebda = *((short *)0x40E);    // get pointer
    ebda = ebda * 0x10 & 0x000FFFFF; // transform segment into linear address

    // search Extended BIOS Data Area for the Root System Description Pointer signature
    for (addr = (u32 *)ebda; (u64)addr < ebda + 1024; addr += 0x10 / sizeof(addr))
    {
        rsdp = acpi_check_rsdptr(addr);
        if (rsdp != NULL)
            return rsdp;
    }
    return NULL;
}

static void acpi_parse_apic(acpi_madt_t *m)
{
    madt = m;
    LOGK("local apic address %#x\n", m->local_apic_addr);
    assert(m->local_apic_addr == local_apic_addr);

    u8 *p = (u8 *)(madt + 1);
    u8 *end = (u8 *)madt + madt->header.length;

    while (p < end)
    {
        apic_header_t *header = (apic_header_t *)p;
        u8 type = header->type;
        u8 length = header->length;

        switch (type)
        {
        case APIC_TYPE_LOCAL:
            apic_local_t *local = (apic_local_t *)p;
            LOGK("found CPU: %#x %#x %#x\n",
                 local->acpi_processor_id, local->acpi_id, local->flags);
            if (cpu_count < MAX_CPU_COUNT)
            {
                cpu_apic_ids[cpu_count++] = local->acpi_id;
            }
            break;
        case APIC_TYPE_IO:
            apic_io_t *io = (apic_io_t *)p;
            LOGK("found I/O APIC: %#x %#x %#x\n",
                 io->io_apic_id, io->io_apic_addr, io->global_base);
            assert(io->io_apic_addr == io_apic_addr);
            break;
        case APIC_TYPE_OVERRIDE:
            apic_override_t *over = (apic_override_t *)p;
            LOGK("apic interrupt override: %d %d %d 0x%04x\n",
                 over->bus, over->source, over->interrupt, over->flags);
            break;
        default:
            LOGK("Unknown apic structure %d\n", type);
            break;
        }
        p += length;
    }
}

static void acpi_parse_dt(acpi_header_t *header)
{
    u32 signature = header->signature;

    char str[5];
    memcpy(str, &signature, 4);
    str[4] = 0;

    LOGK("%s %#x\n", str, signature);
    if (signature == 0x43495041)
    {
        acpi_parse_apic((acpi_madt_t *)header);
    }
}

static void acpi_parse_rsdt(acpi_header_t *rsdt)
{
    u32 *p = (u32 *)(rsdt + 1);
    u32 *end = (u32 *)((u8 *)rsdt + rsdt->length);

    while (p < end)
    {
        u32 address = *p++;
        acpi_parse_dt((acpi_header_t *)(u64)address);
    }
}

static void acpi_parse_rsdp(rsdptr_t *rsdp)
{
    LOGK("acpi find rsdptr %#x\n", rsdp);
    switch (rsdp->revision)
    {
    case 0:
        acpi_parse_rsdt((acpi_header_t *)rsdp->rsdt_addr);
        break;
    case 2:
        // TODO: VERSION 2
        break;
    default:
        LOGK("Unknown ACPI version %d\n", rsdp->revision);
        break;
    }
}

u32 acpi_remap_irq(u32 irq)
{
    acpi_madt_t *m = madt;

    u8 *p = (u8 *)(m + 1);
    u8 *end = (u8 *)m + m->header.length;

    while (p < end)
    {
        apic_override_t *over = (apic_override_t *)p;
        if (over->header.type == APIC_TYPE_OVERRIDE && over->source == irq)
        {
            return over->interrupt;
        }
        p += over->header.length;
    }
    return irq;
}

void acpi_init()
{
    LOGK("acpi init...\n");

    rsdptr_t *rsdp = acpi_find_rsdptr();
    if (!rsdp)
    {
        panic("acpi search rsdptr failure...");
        return;
    }

    ards_t *zone = memory_area((u64)rsdp->rsdt_addr);
    if (!zone)
    {
        panic("acpi memory zone search failure...");
        return;
    }

    map_area(zone->base, zone->size);
    acpi_parse_rsdp(rsdp);
}