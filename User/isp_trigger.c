/**
 * @file isp_trigger.c
 * @brief STC8H ISP触发模块实现
 * @note 完全独立，仅依赖串口基础接口
 */
#include "STC8H.h"
#include "type_def.h"
#include "uart.h"
#include "isp_trigger.h"

// 静态变量：口令匹配状态（模块内私有）
static const uint8_t isp_cmd[] = ISP_TRIGGER_CMD;  // 触发口令常量
static uint8_t isp_match_idx = 0;                  // 口令匹配索引

/**
 * @brief 初始化ISP触发模块
 */
void isp_trigger_init(void) {
    isp_match_idx = 0;  // 重置匹配索引
    // 发送初始化提示（可选，方便调试）
    uart_sendstr((uint8_t*)"ISP Trigger Module Init OK!\r\n");
}

/**
 * @brief 触发STC8H进入ISP模式（核心函数）
 */
static void isp_enter(void) {
    // 发送触发成功提示（确保发送完成）
    uart_sendstr((uint8_t*)"ISP Trigger OK! Enter ISP Mode...\r\n");

    // 关闭总中断，防止干扰ISP触发
    EA = 0;
    // STC8H核心指令：触发ISP模式（无需断电）
    IAP_CONTR = 0x60;
}

/**
 * @brief 轮询检测ISP触发口令
 */
void isp_trigger_check(void) {
    uint8_t recv_dat;

    // 串口缓冲区无数据，直接返回
    if (!uarthasdata()) {
        return;
    }

    // 读取串口缓冲区一个字节
    recv_dat = uart_recv();

    // 口令匹配逻辑
    if (recv_dat == isp_cmd[isp_match_idx]) {
        // 当前字节匹配，索引+1
        isp_match_idx++;
        // 口令完整匹配，触发ISP
        if (isp_match_idx >= ISP_CMD_LEN) {
            isp_enter();          // 进入ISP模式
            isp_match_idx = 0;    // 重置索引（防止重复触发）
        }
    } else {
        // 匹配失败，重置索引
        isp_match_idx = 0;
        // 特殊处理：当前字节是口令首字符，重新开始匹配
        if (recv_dat == isp_cmd[0]) {
            isp_match_idx = 1;
        }
    }
}
