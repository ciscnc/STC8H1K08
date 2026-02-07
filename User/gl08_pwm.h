/**
 * @file gl08_pwm.h
 * @brief PWM模块头文件，实现PWM输出控制和输入捕获
 *
 * @date 2026-02-07
 */
#ifndef __GL08_PWM_H__
#define __GL08_PWM_H__

#include "gl08_config.h"

// PWM捕获未完成标志
#define PWM_CAPTURE_NOT_READY  0xFFFF

// PWM捕获通道枚举定义
typedef enum { 
    INPUT_PWM1 = 0, 
    INPUT_PWM2, 
    MAX_PWM_CHANNEL 
} pwm_capture_channel_t;

/**
 * @brief 初始化PWM模块
 */
void pwm_init(void);

/**
 * @brief 初始化PWM输出模式
 */
void pwmb_oc_init(void);

/**
 * @brief 初始化PWM输入捕获模式
 */
void pwma_ic_init(void);

/**
 * @brief 设置PWM输出占空比
 *
 * @param channel PWM通道 (D1或D2)
 * @param duty 占空比值 (0 ~ PWM_FREQUENCY)
 */
void set_pwm_duty(uint8_t channel, uint16_t duty);

/**
 * @brief 获取PWM输入捕获的占空比值
 *
 * @param channel PWM通道 (PWM1或PWM2)
 * @return 占空比值，捕获未完成返回PWM_CAPTURE_NOT_READY
 */
uint16_t get_pwm_ic_duty(uint8_t channel);

/**
 * @brief 启动PWM1输入捕获 (P1.0引脚)
 */
void pwma_ic1_start(void);

/**
 * @brief 停止PWM1输入捕获
 */
void pwma_ic1_stop(void);

/**
 * @brief 启动PWM2输入捕获 (P1.4引脚)
 */
void pwma_ic2_start(void);

/**
 * @brief 停止PWM2输入捕获
 */
void pwma_ic2_stop(void);

/**
 * @brief PWM输入捕获中断服务函数
 */
void pwm_ic_isr(void);

#endif /* __GL08_PWM_H__ */
