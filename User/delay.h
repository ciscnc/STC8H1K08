/*
 * STC8H1K08 系统延时功能头文件
 * 包含毫秒（ms）、微秒（us）延时
 */

#ifndef __DELAY_H__
#define __DELAY_H__

#include "gl08_config.h"

void delay_ms(uint16_t ms);
void delay_us(uint16_t us);

#endif // __DELAY_H__