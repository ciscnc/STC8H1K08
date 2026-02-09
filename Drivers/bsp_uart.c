/**
 * @file bsp_uart.c
 * @brief 串口通信功能实现
 * STC8H1K08 串口通信功能实现，使用中断的方式进行发送和接收；包含8位，16位，字符串发送函数
 */

#include "STC8H.h"
#include "type_def.h"
#include "bsp_uart.h"

// 接收缓冲区与指针
#define UART_BUF_SIZE 16
static uint8_t uart_buf[UART_BUF_SIZE];
static uint8_t rptr = 0;           // 接收写入指针
static uint8_t wptr = 0;           // 接收读取指针（若需FIFO可扩展）
static volatile uint8_t busy = 0;  // 发送忙标志

// 新增：串口临界区函数（保护缓冲区读写）
static void uart_enter_critical(void) {
    ES = 0;  // 关闭串口中断，防止改写缓冲区
}

static void uart_exit_critical(void) {
    ES = 1;  // 恢复串口中断
}

// UART 初始化
void uart_init(void) {
    // 配置串口模式：模式1，可变波特率8位数据，允许接收
    SCON = 0x50;  // 0x50 = 0101 0000，REN=1（允许接收），SM1=1（模式1）

    AUXR &= ~(1 << 4);  // Timer2 stop
    AUXR |= 0x01;       // 使用 Timer2 作为波特率发生器
    AUXR &= ~(1 << 3);  // Timer2 set As Timer
    AUXR |= (1 << 2);   // Timer2 设置为 1T 模式
    T2L = BRT;          // 波特率发生器数，Timer2
    T2H = BRT >> 8;
    IE2 &= ~(1 << 2);  // 禁止 Timer2 中断
    AUXR |= (1 << 4);  // 使能 Timer2

    ES = 1;  // 使能串口1中断
}

#if UART_PRINT

// UART 中断服务函数
void uart_isr(void) interrupt 4 {
    if (TI) {  // 发送中断（数据发送完成）
        TI = 0;
        busy = 0;  // 发送完成，释放忙标志
    }
    if (RI) {  // 接收中断（数据接收完成）
        RI = 0;
        uart_buf[rptr++] = SBUF;  // 数据存入缓冲区
        rptr %= UART_BUF_SIZE;    // 循环缓冲区，防止溢出
    }
}

// 单字节发送（中断方式，非阻塞）
void uart_send(uint8_t dat) {
    while (busy)
        ;        // 等待上一次发送完成
    busy = 1;    // 标记为忙
    SBUF = dat;  // 写入数据到发送缓冲
}

// 8位无符号数发送（0~255）
void uart_uint8(uint8_t dat) {
    if (dat >= 100) {
        uart_send(dat / 100 + '0');
        dat %= 100;
    }
    if (dat >= 10) {
        uart_send(dat / 10 + '0');
        dat %= 10;
    }
    uart_send(dat + '0');
}

// 16位无符号数发送（0~65535）
void uart_uint16(uint16_t dat) {
    uint8_t started = 0;
    uint16_t div = 10000;

    do {
        uint8_t digit = dat / div;
        dat %= div;
        if (digit || started || div == 1) {
            uart_send(digit + '0');
            started = 1;
        }
        div /= 10;
    } while (div);
}

// 8位无符号数16进制发送
void uart_hex8(uint8_t dat) {
    uint8_t nibble;

    nibble = (dat >> 4) & 0x0F;
    uart_send(nibble + (nibble < 10 ? '0' : 'A' - 10));

    nibble = dat & 0x0F;
    uart_send(nibble + (nibble < 10 ? '0' : 'A' - 10));
}

// 打印标签+16位数值+换行
void uart_print_u16(const uint8_t *label, uint16_t value) {
    uart_sendstr(label);
    uart_uint16(value);
    uart_sentEnter();
}

// 打印标签+8位数值+换行
void uart_print_u8(const uint8_t *label, uint8_t value) {
    uart_sendstr(label);
    uart_uint8(value);
    uart_sentEnter();
}

// 换行符发送
void uart_sentEnter(void) {
    uart_send('\r');
    uart_send('\n');
}

// 字符串发送（循环调用单字节发送）
void uart_sendstr(const uint8_t *str) {
    while (*str) {
        uart_send(*str++);
    }
}

// 查询方式：读取接收缓冲区（非中断场景可用）
uint8_t uart_recv(void) {
    uint8_t dat = 0;
    uart_enter_critical();  // 临界区仅包裹"判断+读数据+改指针"
    if (rptr != wptr) {
        dat = uart_buf[wptr++];
        wptr %= UART_BUF_SIZE;
    }
    uart_exit_critical();
    return dat;
}

// 中断方式：判断缓冲区是否有数据（供上层逻辑轮询）
uint8_t uarthasdata(void) {
    uint8_t ret;
    uart_enter_critical();  // 仅关串口中断，几十纳秒
    ret = (rptr != wptr);
    uart_exit_critical();
    return ret;
}

#endif  // UART_PRINT
