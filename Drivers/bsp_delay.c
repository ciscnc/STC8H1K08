/**
 * @file bsp_delay.c
 * @brief 系统延时功能实现
 *
 * @date 2026-02-07
 */
#include "bsp_delay.h"
#include "STC8H.h"

// 微秒延时，单位：us
void delay_us(uint16_t us) {
    while (us--) {
        // 24MHz主频，1机器周期 = 12/24 = 0.5us
        // 延时1us约需2个NOP指令，使用STC8H.H中的NOP4()宏
        NOP4();
    }
}

// 毫秒延时，单位：ms
void delay_ms(uint16_t ms) {
    while (ms--) {
        delay_us(1000);
    }
}
