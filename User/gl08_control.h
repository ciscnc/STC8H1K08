/**
 * @file gl08_control.h
 * @brief 控制逻辑头文件，实现调光控制、模式切换等核心功能
 *
 * @date 2026-02-07
 */
#ifndef __GL08_CONTROL_H__
#define __GL08_CONTROL_H__

#include "gl08_config.h"

/**
 * @brief 控制逻辑初始化函数
 */
void control_init(void);

/**
 * @brief 第一次启动ADC转换和输入捕获
 */
void first_start_conversion(void);

/**
 * @brief 控制逻辑主任务函数，包含调光控制、模式切换等核心控制功能
 */
void control_task(void);

#endif /* __GL08_CONTROL_H__ */
