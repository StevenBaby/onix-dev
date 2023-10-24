#include <onix/types.h>
#include <onix/descriptor.h>
#include <onix/string.h>
#include <onix/printk.h>
#include <onix/debug.h>
#include <onix/assert.h>
#include <onix/interrupt.h>
#include <onix/apic.h>
#include <onix/pic.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

typedef struct gate_t
{
    u16 offset0;     // 段内偏移 0 ~ 15 位
    u16 selector;    // 代码段选择子
    u8 ist : 3;      // IST 机制
    u8 RESERVED : 5; // 保留不用
    u8 type : 4;     // 任务门/中断门/陷阱门
    u8 segment : 1;  // segment = 0 表示系统段
    u8 DPL : 2;      // 使用 int 指令访问的最低权限
    u8 present : 1;  // 是否有效
    u16 offset1;     // 段内偏移 16 ~ 31 位
    u32 offset2;     // 段内偏移 32 ~ 63 位
    u32 RESERVED;    // 保留
} _packed gate_t;

#define IDT_SIZE 256
#define ENTRY_SIZE 0x30

static gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

u64 handler_table[IDT_SIZE];
extern u64 handler_entry_table[IDT_SIZE];

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

void exception_handler(
    u64 _rdi, u64 _rsi, u64 _rdx, u64 _rcx, u64 _r8, u64 _r9,
    u64 gs, u64 fs, u64 es, u64 ds,
    u64 r15, u64 r14, u64 r13, u64 r12, u64 r11, u64 r10, u64 r9, u64 r8,
    u64 rdi, u64 rsi, u64 rbx, u64 rdx, u64 rcx, u64 rax, u64 rbp,
    u64 vector, u64 error, u64 rip, u64 cs, u64 eflags)
{
    char *message = NULL;
    if (vector < 22)
        message = messages[vector];
    else
        message = messages[15];

    printk("\nEXCEPTION : %s \n", message);
    printk("   VECTOR : 0x%02X\n", vector);
    printk("    ERROR : 0x%016X\n", error);
    printk("   EFLAGS : 0x%016X\n", eflags);
    printk("       CS : 0x%016X\n", cs);
    printk("      RIP : 0x%016X\n", rip);

    bool hanging = true;

    // 阻塞
    while (hanging)
        ;

    // 通过 EIP 的值应该可以找到出错的位置
    // 也可以在出错时，可以将 hanging 在调试器中手动设置为 0
    // 然后在下面 return 打断点，单步调试，找到出错的位置
    return;
}

// 默认中断处理函数，用于占位
void default_handler(
    u64 _rdi, u64 _rsi, u64 _rdx, u64 _rcx, u64 _r8, u64 _r9,
    u64 gs, u64 fs, u64 es, u64 ds,
    u64 r15, u64 r14, u64 r13, u64 r12, u64 r11, u64 r10, u64 r9, u64 r8,
    u64 rdi, u64 rsi, u64 rbx, u64 rdx, u64 rcx, u64 rax, u64 rbp,
    u64 vector, u64 error, u64 rip, u64 cs, u64 eflags)
{
    DEBUGK("[%x] default interrupt called...\n", vector);
}

void idt_init()
{
    LOGK("idt init...\n");

    memset(idt, 0, sizeof(idt));
    memset(handler_table, 0, sizeof(handler_table));
    for (size_t i = 0; i < IDT_SIZE; i++)
        handler_table[i] = (u64)default_handler;

    for (size_t i = 0; i < IDT_SIZE; i++)
    {
        gate_t *gate = &idt[i];
        u64 handler = handler_entry_table[i];

        gate->offset0 = handler & 0xffff;
        gate->offset1 = (handler >> 16) & 0xffff;
        gate->offset2 = (handler >> 32) & 0xffffffff;

        gate->selector = 1 << 3; // 代码段
        gate->type = 0b1110;     // 中断门
        gate->segment = 0;       // 系统段
        gate->DPL = 0;           // 内核态
        gate->present = 1;       // 有效
    }

    for (size_t i = 0; i < 0x20; i++)
    {
        handler_table[i] = (u64)exception_handler;
    }

    idt_ptr.base = (u64)idt;
    idt_ptr.limit = sizeof(idt) - 1;
    asm volatile("lidt idt_ptr\n");
    // BMB;
}

void send_eoi(int vector)
{
    if (vector < 0x20 || vector >= 0x30)
        return;
    if (apic_valid)
        apic_send_eoi(vector);
    else
        pic_send_eoi(vector);
}

void set_interrupt_mask(u32 irq, bool enable)
{
    if (apic_valid)
        apic_interrupt_mask(irq, enable);
    else
        pic_interrupt_mask(irq, enable);
}

// 注册中断处理函数
void set_interrupt_handler(u32 vector, void *handler)
{
    assert(vector >= 0 && vector < IDT_SIZE);
    handler_table[vector] = (u64)handler;
}

// 清除 IF 位，返回设置之前的值
bool interrupt_disable()
{
    asm volatile(
        "pushf\n"         // 将当前 eflags 压入栈中
        "cli\n"           // 清除 IF 位，此时外中断已被屏蔽
        "popq %rax\n"     // 将刚才压入的 eflags 弹出到 eax
        "shrq $9, %rax\n" // 将 eax 右移 9 位，得到 IF 位
        "andq $1, %rax\n" // 只需要 IF 位
    );
}

// 获得 IF 位
bool get_interrupt_state()
{
    asm volatile(
        "pushf\n"         // 将当前 eflags 压入栈中
        "popq %rax\n"     // 将压入的 eflags 弹出到 eax
        "shrq $9, %rax\n" // 将 eax 右移 9 位，得到 IF 位
        "andq $1, %rax\n" // 只需要 IF 位
    );
}

// 设置 IF 位
void set_interrupt_state(bool state)
{
    if (state)
        asm volatile("sti\n");
    else
        asm volatile("cli\n");
}

void interrupt_init()
{
    idt_init();
}