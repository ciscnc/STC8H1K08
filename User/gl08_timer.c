/**
 * @file gl08_timer.c
 * @brief 定时器模块实现
 *
 * @date 2026-02-07
 */
#include "STC8H.h"
#include "gl08_timer.h"
#include "task.h"

// Timer 配置
#define TIMER0_RELOAD_H ((65536 - FOSC / 12 / 2000) >> 8)  // Timer0 2ms 定时器
#define TIMER0_RELOAD_L ((65536 - FOSC / 12 / 2000) & 0xFF)

#define TIMER1_RELOAD_H ((65536 - FOSC / 12 / 1000) >> 8)  // Timer1 1ms 定时器
#define TIMER1_RELOAD_L ((65536 - FOSC / 12 / 1000) & 0xFF)

// Timer 初始化
void timer_init(void) {
    // Timer1 为 1ms 触发溢出中断
    TMOD = 0x00;  // Timer1 works in mode 0

    TL1 = TIMER1_RELOAD_H;
    TH1 = TIMER1_RELOAD_L;

    // Enable Timer1 interrupt
    ET1 = 1;

    // Start Timer1
    TR1 = 1;
}

// Timer1 中断服务函数
void Timer1_ISR(void) interrupt 3 {
    TF1 = 0;                        // 清除定时器1溢出中断标志
    Task_Marks_Handler_Callback();  // 调用任务标记回调函数
}
