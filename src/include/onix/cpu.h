#ifndef ONIX_CPU_H
#define ONIX_CPU_H

#include <onix/types.h>

// https://en.wikipedia.org/wiki/Control_register
// https://wiki.osdev.org/CPU_Registers_x86-64

enum
{
    CR0_PE = 1 << 0,  // Protection Enable 启用保护模式
    CR0_MP = 1 << 1,  // Monitor Coprocessor
    CR0_EM = 1 << 2,  // Emulation 启用模拟，表示没有 FPU
    CR0_TS = 1 << 3,  // Task Switch 任务切换，延迟保存浮点环境
    CR0_ET = 1 << 4,  // Extension Type 保留
    CR0_NE = 1 << 5,  // Numeric Error 启用内部浮点错误报告
    CR0_WP = 1 << 16, // Write Protect 写保护（禁止超级用户写入只读页）帮助写时复制
    CR0_AM = 1 << 18, // Alignment Mask 对齐掩码
    CR0_NW = 1 << 29, // Not Write-Through 不是直写
    CR0_CD = 1 << 30, // Cache Disable 禁用内存缓冲
    CR0_PG = 1 << 31, // Paging 启用分页

    CR4_VME = 1 << 0,         // Virtual 8086 Mode Extensions
    CR4_PVI = 1 << 1,         // Protected-mode Virtual Interrupts
    CR4_TSD = 1 << 2,         // Time Stamp Disable
    CR4_DE = 1 << 3,          // Debugging Extensions
    CR4_PSE = 1 << 4,         // Page Size Extension
    CR4_PAE = 1 << 5,         // Physical Address Extension
    CR4_MCE = 1 << 6,         // Machine Check Exception
    CR4_PGE = 1 << 7,         // Page Global Enabled
    CR4_PCE = 1 << 8,         // Performance-Monitoring Counter enable
    CR4_OSFXSR = 1 << 9,      // Operating system support for FXSAVE and FXRSTOR instructions
    CR4_OSXMMEXCPT = 1 << 10, // Operating System Support for Unmasked SIMD Floating-Point Exceptions
    CR4_UMIP = 1 << 11,       // User-Mode Instruction Prevention
    CR4_LA57 = 1 << 12,       // 57-Bit Linear Addresses
    CR4_VMXE = 1 << 13,       // Virtual Machine Extensions Enable
    CR4_SMXE = 1 << 14,       // Safer Mode Extensions Enable
    CR4_FSGSBASE = 1 << 16,   // FSGSBASE Enable
    CR4_PCIDE = 1 << 17,      // PCID Enable
    CR4_OSXSAVE = 1 << 18,    // XSAVE and Processor Extended States Enable
    CR4_KL = 1 << 19,         // Key Locker Enable
    CR4_SMEP = 1 << 20,       // Supervisor Mode Execution Protection Enable
    CR4_SMAP = 1 << 21,       // Supervisor Mode Access Prevention Enable
    CR4_PKE = 1 << 22,        // Protection Key Enable
    CR4_CET = 1 << 23,        // Control-flow Enforcement Technology
    CR4_PKS = 1 << 24,        // Enable Protection Keys for Supervisor-Mode Pages
    CR4_UINTR = 1 << 25       // User Interrupts Enable
};

typedef struct cpuid_t
{
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
} _packed cpuid_t;

void cpuid(u32 main, u32 sub, cpuid_t *id);

#define halt() asm volatile("hlt\n");

#endif