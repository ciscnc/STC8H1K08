/**
 * @file gl08_adc.h
 * @brief ADC模块头文件，实现ADC采样和电压转换
 *
 * @date 2026-02-07
 */
#ifndef __GL08_ADC_H__
#define __GL08_ADC_H__

#include "gl08_config.h"

// ADC通道枚举定义
typedef enum
{
    BAND_K1_ADC_CHANNEL = 0,
    BAND_K2_ADC_CHANNEL,
    POWER_ADC_CHANNEL,
    MAX_ADC_CHANNEL
} adc_channel_t;

// ADC转换未完成返回值宏定义
#define ADC_NOT_READY  0xFFFF

/**
 * @brief ADC 初始化
 */
void adc_init(void);

/**
 * @brief 转换 ADC 采样值为电压值，单位：mV
 * 
 * @param adc_val ADC采样值
 * @return uint16_t 转换后的电压值，单位：mV
 */
uint16_t adc_to_voltage(uint16_t adc_val);

/**
 * @brief 启动一轮ADC转换：4轮，每轮3个通道依次转换
 */
void adc_start_conversion(void);

/**
 * @brief 获取ADC转换完成的raw值，转换完成返回值，未完成返回ADC_NOT_READY
 * 
 * @param channel ADC通道枚举
 * @return uint16_t ADC采样原始值，单位：0-1023；如果转换未完成则返回ADC_NOT_READY
 */
uint16_t adc_get_raw_value(adc_channel_t channel);

/**
 * @brief ADC转换完成中断服务函数
 */
void adc_Isr(void);



#endif /* __GL08_ADC_H__ */
