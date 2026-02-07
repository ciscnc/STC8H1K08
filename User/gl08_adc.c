/**
 * @file gl08_adc.c
 * @brief ADC模块实现
 *
 * @date 2026-02-07
 */
#include "STC8H.h"
#include "gl08_adc.h"
#include "delay.h"
#include "uart.h"

// ADC采样值存储
static volatile uint16_t adc_raw_values[3] = {0, 0, 0};

// ADC控制结构体
typedef struct {
    uint16_t temp_values[4][3];  // 临时采样数组，4轮×3通道
    uint8_t  round;             // 当前采样轮次
    uint8_t  channel;           // 当前采样通道
    uint8_t  converting;        // 转换完成标志
} adc_control_t;

static adc_control_t adc_ctrl = {0};

// ADC通道映射表，方便根据枚举获取对应的ADC通道号
static const uint8_t adc_channel_mapping[MAX_ADC_CHANNEL] = {
    BAND_SWITCH_1,  // BAND_K1_ADC_CHANNEL
    BAND_SWITCH_2,  // BAND_K2_ADC_CHANNEL
    POWER_SWITCH   // POWER_ADC_CHANNEL
};

// ADC 初始化
void adc_init(void) {
    ADCCFG = 0x2F;     // 结果右对齐，时钟为16分频：SYSclk/2/16
    ADC_CONTR = 0x80;  // 使能 ADC 模块
    EADC = 1;          // 使能 ADC 中断

    delay_ms(10);  // 延时等待电源稳定
}

// 转换 ADC 采样值为电压值，单位：mV
uint16_t adc_to_voltage(uint16_t adc_val) {
    // 5V reference voltage, 10-bit ADC
    return (uint16_t)((uint32_t)adc_val * 5000 / ADC_RESOLUTION);
}

// 启动一轮ADC转换：4轮，每轮3个通道依次转换
void adc_start_conversion(void) {
    uint8_t i, j;

    adc_ctrl.round = 0;
    adc_ctrl.channel = 0;
    adc_ctrl.converting = 1;

    // 清零临时数组
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            adc_ctrl.temp_values[i][j] = 0;
        }
    }

    // 启动第1轮第1个通道转换
    ADC_CONTR &= 0xF0;
    ADC_CONTR |= adc_channel_mapping[adc_ctrl.channel];
    ADC_CONTR |= 0x40;
}

// 获取ADC转换完成的raw值，转换完成返回值，未完成返回ADC_NOT_READY
uint16_t adc_get_raw_value(adc_channel_t channel) {
    if (adc_ctrl.converting) {
        return ADC_NOT_READY;
    }
    return adc_raw_values[channel];
}

// ADC 中断服务函数
void adc_Isr(void) interrupt 5 {
    if (ADC_CONTR & 0x20) {
        // 保存采样值
        adc_ctrl.temp_values[adc_ctrl.round][adc_ctrl.channel] = ((uint16_t)ADC_RES << 8) | ADC_RESL;

        // 转换下一个通道
        adc_ctrl.channel++;
        if (adc_ctrl.channel >= MAX_ADC_CHANNEL) {
            adc_ctrl.channel = 0;
            adc_ctrl.round++;
        }

        // 检查是否完成4轮转换
        if (adc_ctrl.round >= 4) {
            uint8_t i, j;
            uint32_t sum;
            // 计算各通道平均值
            for (i = 0; i < MAX_ADC_CHANNEL; i++) {
                sum = 0;
                for (j = 0; j < 4; j++) {
                    sum += adc_ctrl.temp_values[j][i];
                }
                adc_raw_values[i] = (uint16_t)(sum >> 2);
            }
            adc_ctrl.converting = 0;
        } else {
            // 启动下一个通道转换
            ADC_CONTR &= 0xF0;
            ADC_CONTR |= adc_channel_mapping[adc_ctrl.channel];
            ADC_CONTR |= 0x40;
        }

        ADC_CONTR &= ~0x20;  // 清除中断标志位
    }
}
