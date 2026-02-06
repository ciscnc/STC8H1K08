/*
 * GL08 Timer 模块头文件
 */

#ifndef __GL08_TIMER_H__
#define __GL08_TIMER_H__

#include "gl08_config.h"

// 函数声明
void timer_init(void);
void timer0_isr(void);
void Timer1_ISR(void);

#endif  // __GL08_TIMER_H__
