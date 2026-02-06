/*
 * STC8H1K08 串口通信功能头文件
 * 包含 UART1 硬件初始化和相关操作函数
 */

#ifndef __UART_H__
#define __UART_H__

#include "gl08_config.h"

// UART 初始化函数
void uart_init(void);

// UART 发送函数
void uart_send(uint8_t dat);
void uart_uint8(uint8_t dat);
void uart_uint16(uint16_t dat);
void uart_sentEnter(void);
void uart_sendstr(const uint8_t *str);

// UART 接收函数
uint8_t uart_recv(void);
uint8_t uarthasdata(void);

#endif  // __UART_H__
