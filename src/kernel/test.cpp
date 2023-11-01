#include <onix/types.h>
#include <onix/debug.h>
#include <list>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

struct element_t
{
    u32 data;
};

element_t item[10];


namespace arch
{
    std::list<element_t> element_list;
    void test_init()
    {
        LOGK("test init...\n");
        new(&element_list) std::list<element_t>;

        for (size_t i = 0; i < 10; i++)
        {
            element_list.push_back(std::move(item[i]));
        }
        
        auto iter = element_list.begin();
        LOGK("%#x %#x\n", iter, &element_list);
    }
}