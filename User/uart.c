/*
 * STC8H1K08 串口通信功能实现
 * 使用中断的方式进行发送和接收；包含8位，16位，字符串发送函数
 */

#include "STC8H.h"
#include "type_def.h"
#include "uart.h"

#if UART_PRINT

// 接收缓冲区与指针
#define UART_BUF_SIZE 16
uint8_t uart_buf[UART_BUF_SIZE];
uint8_t rptr = 0;    // 接收写入指针
uint8_t wptr = 0;    // 接收读取指针（若需FIFO可扩展）
volatile uint8_t busy = 0;    // 发送忙标志


// UART 初始化
void uart_init(void)
{
	
    // 配置串口模式：模式1，可变波特率8位数据，允许接收
    SCON = 0x50;      // 0x50 = 0101 0000，REN=1（允许接收），SM1=1（模式1）
	
		AUXR &= ~(1<<4);	// Timer2 stop
		AUXR |= 0x01;		  // 使用 Timer2 作为波特率发生器
		AUXR &= ~(1<<3);	// Timer2 set As Timer
		AUXR |=  (1<<2);	// Timer2 设置为 1T 模式
		T2L = BRT;        // 波特率发生器数，Timer2
		T2H = BRT >> 8; 
		IE2  &= ~(1<<2);	//禁止 Timer2 中断
		AUXR |=  (1<<4);	//使能 Timer2

    ES = 1;     //使能串口1中断

}

// UART 中断服务函数
void uart_isr(void) interrupt 4
{
    if (TI) {  // 发送中断（数据发送完成）
        TI = 0;
        busy = 0;  // 发送完成，释放忙标志
    }
    if (RI) {  // 接收中断（数据接收完成）
        RI = 0;
        uart_buf[rptr++] = SBUF;  // 数据存入缓冲区
        rptr %= UART_BUF_SIZE;   // 循环缓冲区，防止溢出
    }
}


// 单字节发送（中断方式，非阻塞）
void uart_send(uint8_t dat)
{
    while (busy);  // 等待上一次发送完成
    busy = 1;      // 标记为忙
    SBUF = dat;    // 写入数据到发送缓冲
}

// 8位无符号数发送（目前只能打印0~9）
void uart_uint8(uint8_t dat)
{
		dat += '0';
    while (busy);  // 等待上一次发送完成
    busy = 1;      // 标记为忙
    SBUF = dat;    // 写入数据到发送缓冲
}

// 16位无符号数发送
void uart_uint16(uint16_t dat)
{
	uint16_t temp = 10000;
	uint8_t byte;
	do{
		byte = dat / temp;
		byte += '0';
		uart_send(byte);
		dat %= temp;
		temp /= 10;
	}while(temp);
}

// 换行符发送
void uart_sentEnter(void)
{
  uart_send('\r');
	uart_send('\n');
}

// 字符串发送（循环调用单字节发送）
void uart_sendstr(const uint8_t *str)
{
    while (*str) {
        uart_send(*str++);
    }
}

// 查询方式：读取接收缓冲区（非中断场景可用）
uint8_t uart_recv(void)
{
    uint8_t dat = 0;
    if (rptr != wptr) {  // 缓冲区有数据
        dat = uart_buf[wptr++];
        wptr %= UART_BUF_SIZE;
    }
    return dat;
}

// 中断方式：判断缓冲区是否有数据（供上层逻辑轮询）
uint8_t uarthasdata(void)
{
    return (rptr != wptr);
}


#else  //禁用串口打印时，空实现

#define NOT_SUPPORTED 0

void communication_init(void) {}
void uart_init(void) {}
void uart_isr(void) interrupt 4 {}
void uart_send(uint8_t dat) {}
void uart_uint8(uint8_t dat) {}
void uart_uint16(uint16_t dat) {}
void uart_sentEnter(void) {}
void uart_sendstr(const uint8_t *str){}
uint8_t uart_recv(void) 
{
	return NOT_SUPPORTED;
}
uint8_t uarthasdata(void) 
{
	return NOT_SUPPORTED;
}
	
#endif   //UART_PRINT
