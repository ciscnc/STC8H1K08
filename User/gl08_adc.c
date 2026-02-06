/*
 * GL08 ADC 模块
 * 负责 ADC 采样、滤波和电压转换
 */

#include "STC8H.h"
#include "gl08_adc.h"
#include "delay.h"
#include "uart.h"

// ADC 采样值存储
volatile uint16_t adc_values[3] = {0, 0, 0};

// 辅助函数：冒泡排序
static void bubble_sort(uint16_t arr[], uint8_t n) {
    uint8_t i, j;
    uint16_t temp;

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// ADC 初始化
void adc_init(void) {
    ADCCFG = 0x2F;     // 结果右对齐，时钟为16分频：SYSclk/2/16
    ADC_CONTR = 0x80;  // 使能 ADC 模块
    EADC = 0;          // 失能 ADC 中断，采用查询的方式

    delay_ms(10);  // 延时等待电源稳定
}

// 转换 ADC 采样值为电压值，单位：mV
uint16_t adc_to_voltage(uint16_t adc_val) {
    // Assuming 5V reference voltage, 10-bit ADC
    return (uint16_t)((uint32_t)adc_val * 5000 / ADC_RESOLUTION);
}

// 启动 ADC 转换函数
uint16_t start_adc_conversion(uint8_t adc_chanel, uint8_t convert_count) {
    uint16_t raw_value[5];
    uint8_t i, j;
    uint32_t sum = 0;

    // 检查采样次数是否足够
    if (convert_count < 3) {
        convert_count = 3;  // 至少需要3次采样才能去掉两个最值
    }

    // 初始化数组
    for (i = 0; i < convert_count; i++) {
        raw_value[i] = 0;
    }

    for (i = 0; i < convert_count; i++) {
        ADC_CONTR &= 0xF0;                 // 清除通道选择
        ADC_CONTR |= (0x0F & adc_chanel);  // 选择通道
        ADC_CONTR |= 0x40;                 // 启动转换

        for (j = 0; j < 250; j++) {
            if (ADC_CONTR & 0x20) {
                if (adc_chanel == (ADC_CONTR & 0x0F)) {
                    raw_value[i] = ((uint16_t)ADC_RES << 8) | ADC_RESL;
                }
                ADC_CONTR &= ~0x20;  // 清除中断标志位
            }
        }
    }

    bubble_sort(raw_value, convert_count);  // 对采样值进行排序

    // 去掉最小值和最大值，计算剩余值的平均值
    for (i = 1; i < convert_count - 1; i++) {
        sum += raw_value[i];
    }

    return (uint16_t)(sum / (convert_count - 2));  // 返回平均值
}

// ADC 中断服务函数(这里未使用中断的方式，而是使用查询的方式)
void adc_Isr(void) interrupt 5 {
    // 未使用
}
