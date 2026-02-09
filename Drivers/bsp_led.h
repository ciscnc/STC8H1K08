/**
 * @file bsp_led.h
 * @brief LED模块头文件
 */

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include "type_def.h"

// LED 翻转周期（1s），假设系统滴答为 1ms
#define LED_TOGGLE_PERIOD  1000

// LED 操作宏（P1.2）
#define LED_ON()   (P1 |= (1 << 2))
#define LED_OFF()  (P1 &= ~(1 << 2))
#define LED_TOGGLE() (P1 ^= (1 << 2))

// 函数声明
void led_init(void);
void led_toggle(void);
void led_task(void);

#endif /* __BSP_LED_H__ */
