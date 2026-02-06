/*
 * GL08 PWM 模块头文件
 */

#ifndef __GL08_PWM_H__
#define __GL08_PWM_H__

#include "gl08_config.h"

// 函数声明
void pwm_init(void);
void pwmb_oc_init(void);
void pwma_ic_init(void);
void set_pwm_duty(uint8_t channel, uint16_t duty);
uint16_t get_pwm_ic_duty(uint8_t channel);
void pwma_ic1_start(void);
void pwma_ic1_stop(void);
void pwma_ic2_start(void);
void pwma_ic2_stop(void);
void pwm_ic_isr(void);

// PWM 占空比全局变量声明
extern volatile uint16_t pwm1_duty;
extern volatile uint16_t pwm2_duty;

#endif  // __GL08_PWM_H__
