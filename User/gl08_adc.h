/*
 * GL08 ADC 模块头文件
 */

#ifndef __GL08_ADC_H__
#define __GL08_ADC_H__

#include "gl08_config.h"

// 函数声明
void adc_init(void);
uint16_t adc_to_voltage(uint16_t adc_val);
uint16_t start_adc_conversion(uint8_t adc_chanel, uint8_t convert_count);
void adc_Isr(void);

// ADC 采样值全局变量声明
extern volatile uint16_t adc_values[3];

#endif  // __GL08_ADC_H__
