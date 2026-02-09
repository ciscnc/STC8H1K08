/**
 * @file bsp_adc.h
 * @brief ADC模块头文件，实现ADC采样和电压转换
 *
 * @date 2026-02-07
 */
#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "gl08_config.h"

// ADC寄存器位定义
#define ADC_CONTR_ADIE   0x80   // ADC中断使能位
#define ADC_CONTR_START  0x40   // ADC转换启动位
#define ADC_CONTR_FLAG   0x20   // ADC转换完成标志位
#define ADC_CONTR_CH_MASK 0x0F  // ADC通道选择掩码

#define ADC_CFG_ALIGN_RIGHT 0x00   // 结果右对齐
#define ADC_CFG_ALIGN_LEFT  0x20   // 结果左对齐
#define ADC_CFG_CLK_DIV_16  0x0F   // 时钟16分频

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
 * @param is_enforce
 * 是否强制启动转换，如果为1则无论当前状态如何都重新启动转换；如果为0则仅在当前未转换或转换完成时启动转换
 */
void adc_start_conversion(uint8_t is_enforce);

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



#endif /* __BSP_ADC_H__ */
