#ifndef ONIX_INTERRUPT_H
#define ONIX_INTERRUPT_H

#include <onix/types.h>

#define INTR_DE 0   // 除零错误
#define INTR_DB 1   // 调试
#define INTR_NMI 2  // 不可屏蔽中断
#define INTR_BP 3   // 断点
#define INTR_OF 4   // 溢出
#define INTR_BR 5   // 越界
#define INTR_UD 6   // 指令无效
#define INTR_NM 7   // 协处理器不可用
#define INTR_DF 8   // 双重错误
#define INTR_OVER 9 // 协处理器段超限
#define INTR_TS 10  // 无效任务状态段
#define INTR_NP 11  // 段无效
#define INTR_SS 12  // 栈段错误
#define INTR_GP 13  // 一般性保护异常
#define INTR_PF 14  // 缺页错误
#define INTR_RE1 15 // 保留
#define INTR_MF 16  // 浮点异常
#define INTR_AC 17  // 对齐检测
#define INTR_MC 18  // 机器检测
#define INTR_XM 19  // SIMD 浮点异常
#define INTR_VE 20  // 虚拟化异常
#define INTR_CP 21  // 控制保护异常

void send_eoi(int vector);

// 设置中断处理函数
void set_interrupt_handler(u32 vector, void *handler);
void set_interrupt_mask(u32 irq, bool enable);

bool interrupt_disable();             // 清除 IF 位，返回设置之前的值
bool get_interrupt_state();           // 获得 IF 位
void set_interrupt_state(bool state); // 设置 IF 位

#endif