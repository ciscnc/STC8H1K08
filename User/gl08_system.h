/**
 * @file gl08_system.h
 * @brief 系统初始化头文件，实现系统和硬件初始化
 *
 * @date 2026-02-07
 */
#ifndef __GL08_SYSTEM_H__
#define __GL08_SYSTEM_H__

#include "type_def.h"

/**
 * @brief 系统初始化函数，执行系统和硬件的初始化流程
 */
void system_init(void);

/**
 * @brief 硬件初始化函数，配置所有硬件外设
 */
void hardware_init(void);

#endif /* __GL08_SYSTEM_H__ */
