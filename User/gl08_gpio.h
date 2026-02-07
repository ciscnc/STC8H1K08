/**
 * @file gl08_gpio.h
 * @brief GPIO模块头文件，实现GPIO初始化和操作
 *
 * @date 2026-02-07
 */
#ifndef __GL08_GPIO_H__
#define __GL08_GPIO_H__

#include "gl08_config.h"

/**
 * @brief GPIO初始化函数，配置所有GPIO引脚的输入输出模式和初始状态
 */
void gpio_init(void);

#endif /* __GL08_GPIO_H__ */
