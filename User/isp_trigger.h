/**
 * @file isp_trigger.h
 * @brief STC8H ISP触发模块（独立模块，通过串口检测口令触发ISP）
 * @note 口令：@STCISP#，触发后无需断电直接进入ISP下载模式
 */
#ifndef ISP_TRIGGER_H
#define ISP_TRIGGER_H

#include "type_def.h"

// ISP触发口令配置（可自定义）
#define ISP_TRIGGER_CMD    "@STCISP#"  // 触发口令
#define ISP_CMD_LEN        8           // 口令长度（@+STCISP+#=8）

/**
 * @brief 初始化ISP触发模块
 * @note 需在uart_init后调用
 */
void isp_trigger_init(void);

/**
 * @brief 轮询检测ISP触发口令
 * @note 需在主循环中调用，建议每1ms调用一次
 */
void isp_trigger_check(void);

#endif // ISP_TRIGGER_H
