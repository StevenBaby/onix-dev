#ifndef ONIX_ZONE_H
#define ONIX_ZONE_H

#include <onix/types.h>
#include <onix/list.h>

typedef struct zone_t
{
    list_node_t node; // 链表节点
    u64 base;         // 内存基地址
    u64 size;         // 内存长度
} zone_t;

u64 zone_alloc(u8 order);
void zone_free(u64 base, u8 order);

#endif