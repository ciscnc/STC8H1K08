/*
 * GL08 双通道控制板硬件驱动头文件
 * 包含 GPIO, ADC, PWM 和 Timer 硬件初始化和操作函数
 */

#ifndef __GL08_HARDWARE_H__
#define __GL08_HARDWARE_H__

#include "gl08_config.h"

// Hardware initialization functions
void system_init(void);
void hardware_init(void);
void gpio_init(void);
void adc_init(void);
void pwm_init(void);
void pwmb_oc_init(void);
void pwma_ic_init(void);
void timer_init(void);

// ADC operation functions
uint16_t start_adc_conversion(uint8_t adc_chanel, uint8_t convert_count);
uint16_t get_adc_value(uint8_t channel);
uint16_t adc_to_voltage(uint16_t adc_val);

// PWM operation functions
void set_pwm_duty(uint8_t channel, uint16_t value);
uint16_t get_pwm_ic_duty(uint8_t channel);
void pwma_ic1_start(void);
void pwma_ic2_start(void);
void pwma_ic1_stop(void);
void pwma_ic2_stop(void);

// Switch scanning functions
uint8_t read_band_switch(uint8_t band_switch);
uint8_t read_power_switch(void);
void scan_switches(void);


#endif // __GL08_HARDWARE_H__
