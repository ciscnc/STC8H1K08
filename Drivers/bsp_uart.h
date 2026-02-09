/**
 * @file bsp_uart.h
 * @brief 串口通信头文件，实现UART通信功能
 *
 * @date 2026-02-07
 */
#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "gl08_config.h"

// UART 初始化函数

/**
 * @brief UART初始化函数，配置串口通信参数
 */
void uart_init(void);

// UART 发送函数

/**
 * @brief UART发送单个字节
 *
 * @param dat 要发送的数据
 */
void uart_send(uint8_t dat);

/**
 * @brief UART发送uint8_t类型数据
 *
 * @param dat 要发送的数据
 */
void uart_uint8(uint8_t dat);

/**
 * @brief UART发送uint16_t类型数据
 *
 * @param dat 要发送的数据
 */
void uart_uint16(uint16_t dat);

/**
 * @brief UART发送8位十六进制数据
 *
 * @param dat 要发送的数据
 */
void uart_hex8(uint8_t dat);

/**
 * @brief UART打印uint8_t类型数据带标签
 *
 * @param label 标签字符串
 * @param value 数据值
 */
void uart_print_u8(const uint8_t *label, uint8_t value);

/**
 * @brief UART打印uint16_t类型数据带标签
 *
 * @param label 标签字符串
 * @param value 数据值
 */
void uart_print_u16(const uint8_t *label, uint16_t value);

/**
 * @brief UART发送换行符
 */
void uart_sentEnter(void);

/**
 * @brief UART发送字符串
 *
 * @param str 要发送的字符串
 */
void uart_sendstr(const uint8_t *str);

// UART 接收函数

/**
 * @brief UART接收单个字节数据
 *
 * @return uint8_t 接收到的数据
 */
uint8_t uart_recv(void);

/**
 * @brief UART检测是否有数据可读
 *
 * @return uint8_t 0表示无数据，1表示有数据
 */
uint8_t uarthasdata(void);

#endif /* __BSP_UART_H__ */
