/**
 * @file bsp_timer.h
 * @brief 定时器模块头文件，实现定时器初始化和中断处理
 *
 * @date 2026-02-07
 */
#ifndef __BSP_TIMER_H__
#define __BSP_TIMER_H__

#include "gl08_config.h"

/**
 * @brief 定时器初始化函数，配置Timer1工作模式和中断
 */
void timer_init(void);

/**
 * @brief Timer1中断服务函数
 */
void Timer1_ISR(void);

// Timer1 计数变量（用于调试）
extern volatile uint16_t timer1_count;

#endif /* __BSP_TIMER_H__ */
