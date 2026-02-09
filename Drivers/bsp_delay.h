/**
 * @file bsp_delay.h
 * @brief 系统延时功能头文件，包含毫秒和微秒延时函数
 *
 * @date 2026-02-07
 */
#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#include "gl08_config.h"

/**
 * @brief 毫秒级延时函数
 *
 * @param ms 延时时间，单位：毫秒
 */
void delay_ms(uint16_t ms);

/**
 * @brief 微秒级延时函数
 *
 * @param us 延时时间，单位：微秒
 */
void delay_us(uint16_t us);

#endif /* __BSP_DELAY_H__ */
