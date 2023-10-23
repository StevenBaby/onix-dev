
#include <onix/stdlib.h>

// 计算 num 分成 size 的数量
u64 div_round_up(u64 num, u64 size)
{
    return (num + size - 1) / size;
}
