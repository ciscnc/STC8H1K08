/*
 * STC8H1K08 系统延时功能实现
 * 目前只实现毫秒级别的绝对延时函数
 */

#include "delay.h"

// 延时函数，单位：ms （绝对延时）
void delay_ms(uint16_t ms)
{
	unsigned int i;
	do{
		i = FOSC / 10000;
		while(--i);
	}while(--ms);
}