#include <onix/types.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/cpu.h>
#include <onix/printk.h>
#include <onix/string.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD "AuthenticAMD"
#define CPUID_VENDOR_INTEL "GenuineIntel"

static char vendor_info[14];

// 检测是否支持 cpuid 指令
bool cpu_check_cpuid()
{
    bool ret;
    asm volatile(
        "pushfq \n"                            // 保存 eflags
        "pushfq \n"                            // 得到 eflags
        "xorq $0x0000000000200000L, (%%rsp)\n" // 反转 ID 位
        "popfq\n"                              // 写入 eflags
        "pushfq\n"                             // 得到 eflags
        "popq %%rax\n"                         // 写入 rax
        "xorq (%%rsp), %%rax\n"                // 将写入的值与原值比较
        "andq $0x0000000000200000L, %%rax\n"   // 得到 ID 位
        "shrq $21, %%rax\n"                    // 右移 21 位，得到是否支持
        "popfq\n"                              // 恢复 eflags
        : "=a"(ret));
    return ret;
}

void cpuid(u32 main, u32 sub, cpuid_t *id)
{
    asm volatile(
        "cpuid \n"
        : "=a"(id->eax),
          "=b"(id->ebx),
          "=c"(id->ecx),
          "=d"(id->edx)
        : "a"(main), "c"(sub));
}

void cpu_init()
{
    LOGK("cpu init...\n");
    bool valid = cpu_check_cpuid();
    LOGK("cpu check cpuid %d\n", valid);
    assert(valid);

    cpuid_t id;

    cpuid(0, 0, &id);
    memcpy(vendor_info, &id.ebx, 4);
    memcpy(vendor_info + 4, &id.edx, 4);
    memcpy(vendor_info + 8, &id.ecx, 4);

    LOGK("cpu vendor %s max value %d\n", vendor_info, id.eax);

    cpuid(0x80000008, 0, &id);
    LOGK("cpu physical address bits %d\n", id.eax & 0xff);
    LOGK("cpu linear address bits %d\n", (id.eax >> 8) & 0xff);
}