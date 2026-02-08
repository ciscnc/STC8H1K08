/*
 * LED 闪烁模块
 * P1.2 引脚，高电平亮，1s 翻转一次
 */

#include "STC8H.h"
#include "led.h"

// LED 初始化
void led_init(void) {
    // 设置 P1.2 为推挽输出
    P1M0 |= (1 << 2);
    P1M1 &= ~(1 << 2);

    // 初始化为低电平（灭）
    LED_OFF();
}

// LED 翻转
void led_toggle(void) {
    LED_TOGGLE();
}

// LED 任务（由任务系统调度器调用）
void led_task(void) {
    led_toggle();
}
