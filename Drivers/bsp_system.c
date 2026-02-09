/**
 * @file bsp_system.c
 * @brief 系统初始化模块实现
 *
 * @date 2026-02-07
 */
#include "STC8H.h"
#include "bsp_system.h"

// System 初始化
void system_init(void) {
    // 失能硬件看门狗
    WDT_CONTR = 0x00;

    // 中断优先级配置 (0=最低, 1, 2, 3=最高)

    // ADC: 最高优先级 (3)
    IPH |= PADCH;
    PADC = 1;

    // PWM: 次高优先级 (2)
    IP2H |= PPWMAH;
    IP2 &= ~PPWMA;

    // Timer: 优先级1
    IPH &= ~PT1H;
    PT1 = 1;

    // UART: 最低优先级 (0)
    IPH &= ~PSH;
    PS = 0;
}
