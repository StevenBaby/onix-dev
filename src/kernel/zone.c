#include <onix/memory.h>
#include <onix/zone.h>
#include <onix/assert.h>
#include <onix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_SIZE 21

list_t zone_list[ZONE_SIZE];
list_t zone_dma_list;
list_t free_zone_list;

static void base_zone_init()
{
    LOGK("zone size %d\n", sizeof(zone_t));

    list_init(&free_zone_list);
    zone_t *zone = (zone_t *)MEMORY_BASEZONE;
    for (size_t i = 0; i < (PAGE_SIZE / sizeof(zone_t)); i++, zone++)
    {
        list_pushback(&free_zone_list, &zone->node);
    }
}

static zone_t *make_zone(u64 base, u64 size)
{
    if (list_empty(&free_zone_list))
    {
        panic("out of memory!!!");
    }
    zone_t *zone = element_entry(zone_t, node, list_pop(&free_zone_list));
    zone->base = base;
    zone->size = size;
    return zone;
}

static void free_zone(zone_t *zone)
{
    zone->base = 0;
    zone->size = 0;
    list_pushback(&free_zone_list, &zone->node);
}

static void add_zone(u64 base, u64 size)
{
    LOGK("add zone base 0x%p size 0x%p\n", base, size);
    for (int i = ZONE_SIZE - 1; i >= 0; i--)
    {
        u64 zsize = PAGE_SIZE << i;
        while (size >= zsize)
        {
            LOGK("zone size %#p match idx %d zsize %#p\n", size, i, zsize);

            zone_t *zone = make_zone(base, zsize);
            list_insert_sort(&zone_list[i], &zone->node, element_node_offset(zone_t, node, base));

            assert(zone->size == zsize);
            assert(zone->base == base);

            base += zsize;
            size -= zsize;
        }
    }
}

u64 zone_alloc(u8 order)
{
    assert(order < ZONE_SIZE);
    u64 addr = 0;
    u64 size = PAGE_SIZE << order;
    zone_t *zone = NULL;
    if (list_empty(&zone_list[order]))
    {
        addr = zone_alloc(order + 1);
        LOGK("split base %#p size %#p\n", addr, size << 1);

        zone = make_zone(addr, size);
        list_insert_sort(&zone_list[order], &zone->node, element_node_offset(zone_t, node, base));

        zone = make_zone(addr + size, size);
        list_insert_sort(&zone_list[order], &zone->node, element_node_offset(zone_t, node, base));
    }

    zone = element_entry(zone_t, node, list_pop(&zone_list[order]));
    assert(zone->size == size);
    addr = zone->base;
    free_zone(zone);
    return addr;
}

static void zone_merge(zone_t *z1, zone_t *z2, u8 order)
{
    assert(z1->size == z2->size);
    assert(z1->base + z1->size == z2->base);

    u64 base = z1->base;
    u64 size = z1->size << 1;

    LOGK("merge base %#p size %#p\n", base, size);

    list_remove(&z1->node);
    list_remove(&z2->node);
    free_zone(z2);
    free_zone(z1);

    zone_free(base, order + 1);
}

void zone_free(u64 base, u8 order)
{
    u64 size = PAGE_SIZE << order;
    zone_t *zone = make_zone(base, size);
    list_insert_sort(&zone_list[order], &zone->node, element_node_offset(zone_t, node, base));

    if (zone->node.prev != &zone_list[order].head)
    {
        zone_t *zone0 = element_entry(zone_t, node, zone->node.prev);
        if (zone0->base + size == base)
        {
            zone_merge(zone0, zone, order);
            return;
        }
    }

    if (zone->node.next != &zone_list[order].tail)
    {
        zone_t *zone1 = element_entry(zone_t, node, zone->node.next);
        if (zone1->base - size == base)
        {
            zone_merge(zone, zone1, order);
            return;
        }
    }
}

void zone_init()
{
    base_zone_init();
    list_init(&zone_dma_list);
    for (size_t i = 0; i < ZONE_SIZE; i++)
    {
        list_init(&zone_list[i]);
    }

    ards_t *ptr = ards_table;

    for (size_t i = 0; i < ards_count && i < ZONE_SIZE; i++, ptr++)
    {
        LOGK("Memory base 0x%p size 0x%p type %d\n", ptr->base, ptr->size, ptr->type);
        if (ptr->type != ARDS_ZONE_VALID)
        {
            zone_t *zone = make_zone(ptr->base, ptr->size);
            list_pushback(&zone_dma_list, &zone->node);
            continue;
        }
        if (ptr->base < MEMORY_BASE)
            continue;
        add_zone(ptr->base, ptr->size);
    }
}