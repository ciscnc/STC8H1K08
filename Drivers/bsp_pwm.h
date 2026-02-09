/**
 * @file bsp_pwm.h
 * @brief PWM模块头文件，实现PWM输出控制和输入捕获
 *
 * @date 2026-02-07
 */
#ifndef __BSP_PWM_H__
#define __BSP_PWM_H__

#include "STC8H.h"
#include "type_def.h"

// PWM配置常量
#define GL08_CH1 1        // 通道1，对应PWM1、D1
#define GL08_CH2 2        // 通道2，对应PWM2、D2

// PWMB输出配置（用于输出PWM波）
#define PWMB_PSC (24 - 1)       // PWMB时钟预分频系数
#define PWMB_PERIOD 1000         // PWMB周期值，频率=FOSC/(PWMB_PSC+1)/PWMB_PERIOD=1000Hz
#define PWM7_DUTY 500           // PWM7初始占空比，50%
#define PWM8_DUTY 500           // PWM8初始占空比，50%
#define D1 GL08_CH1             // PWM7，端口P3.3
#define D2 GL08_CH2             // PWM8，端口P3.4

// PWMA输入捕获配置（用于输入捕获外部PWM）
#define PWMA_PSC (24 - 1)       // PWMA时钟预分频系数
#define PWM1 GL08_CH1            // PWM1P，端口P1.0
#define PWM2 GL08_CH2            // PWM3P，端口P1.4

#define PWM_FREQUENCY PWMB_PERIOD  // PWM频率 1kHz

// PWM捕获未完成标志
#define PWM_CAPTURE_NOT_READY  0xFFFF

// PWM捕获中断使能位掩码
#define PWM_CC1_IE    0x02   // CC1中断使能位
#define PWM_CC2_IE    0x04   // CC2中断使能位
#define PWM_CC3_IE    0x08   // CC3中断使能位
#define PWM_CC4_IE    0x10   // CC4中断使能位

#define PWM_CC12_IE   (PWM_CC1_IE | PWM_CC2_IE)   // CC1+CC2中断使能
#define PWM_CC34_IE   (PWM_CC3_IE | PWM_CC4_IE)   // CC3+CC4中断使能

// PWM捕获/比较使能位掩码
#define PWM_CC1_EN    0x01   // CC1捕获使能位
#define PWM_CC2_EN    0x10   // CC2捕获使能位
#define PWM_CC3_EN    0x01   // CC3捕获使能位
#define PWM_CC4_EN    0x10   // CC4捕获使能位

#define PWM_CC12_EN   (PWM_CC1_EN | PWM_CC2_EN)   // CC1+CC2捕获使能
#define PWM_CC34_EN   (PWM_CC3_EN | PWM_CC4_EN)   // CC3+CC4捕获使能

// PWM状态标志位掩码
#define PWM_CC1_FLAG  0x02   // CC1中断标志
#define PWM_CC2_FLAG  0x04   // CC2中断标志
#define PWM_CC3_FLAG  0x08   // CC3中断标志
#define PWM_CC4_FLAG  0x10   // CC4中断标志

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

#endif /* __BSP_PWM_H__ */
