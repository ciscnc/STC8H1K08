/**
 * @file bsp_adc.c
 * @brief ADC模块实现
 *
 * @date 2026-02-07
 */
#include "STC8H.h"
#include "bsp_adc.h"
#include "bsp_delay.h"

// ADC采样值存储
static data volatile uint16_t adc_raw_values[3] = {0, 0, 0};

// ADC控制结构体
typedef struct {
    uint16_t temp_values[4][3];  // 临时采样数组，4轮×3通道
    uint8_t  round;             // 当前采样轮次
    uint8_t  channel;           // 当前采样通道
    uint8_t  converting;        // 转换完成标志
} adc_control_t;

static data adc_control_t adc_ctrl = {0};

// ADC通道映射表，方便根据枚举获取对应的ADC通道号
static data const uint8_t adc_channel_mapping[MAX_ADC_CHANNEL] = {
    BAND_SWITCH_1,  // BAND_K1_ADC_CHANNEL
    BAND_SWITCH_2,  // BAND_K2_ADC_CHANNEL
    POWER_SWITCH   // POWER_ADC_CHANNEL
};

// 保存 ADC 中断使能状态
static data uint8_t adc_ie_backup;

/**
 * @brief 进入ADC临界区（仅关闭ADC中断，不影响其他中断）
 */
static void adc_enter_critical(void) {
    adc_ie_backup = EADC;  // 保存当前ADC中断状态
    EADC = 0;  // 关闭ADC中断
}

/**
 * @brief 退出ADC临界区（恢复ADC中断状态）
 */
static void adc_exit_critical(void) {
    EADC = adc_ie_backup;  // 恢复之前保存的ADC中断状态
}

// ADC 初始化
void adc_init(void) {
    ADCCFG = ADC_CFG_ALIGN_RIGHT | ADC_CFG_CLK_DIV_16;  // 结果右对齐，时钟为16分频：SYSclk/2/16
    ADC_CONTR = ADC_CONTR_ADIE;                          // 使能 ADC 模块
    EADC = 1;                                            // 使能 ADC 中断

    delay_ms(10);  // 延时等待电源稳定
}

// 转换 ADC 采样值为电压值，单位：mV
uint16_t adc_to_voltage(uint16_t adc_val) {
    // 5V reference voltage, 10-bit ADC
    return (uint16_t)((uint32_t)adc_val * 5000 / ADC_RESOLUTION);
}

// 启动一轮ADC转换：4轮，每轮3个通道依次转换
void adc_start_conversion(uint8_t is_enforce) {
    uint8_t i, j;
    uint8_t need_start = 1;  // 标记是否需要启动转换

    // 核心修正：所有对converting的读/写都包在临界区
    adc_enter_critical();
    if (!is_enforce) {
        // 非强制模式：检查是否正在转换
        if (adc_ctrl.converting) {
            need_start = 0;  // 正在转换，无需启动
        } else {
            // 不在转换中，初始化控制参数
            adc_ctrl.round = 0;
            adc_ctrl.channel = 0;
            adc_ctrl.converting = 1;
        }
    } else {
        // 强制模式：不管是否在转换，都重置参数
        adc_ctrl.round = 0;
        adc_ctrl.channel = 0;
        adc_ctrl.converting = 1;
    }
    adc_exit_critical();

    // 如果不需要启动，直接返回
    if (!need_start) {
        return;
    }

    // 清零临时数组
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            adc_ctrl.temp_values[i][j] = 0;
        }
    }

    // 启动第1轮第1个通道转换
    EADC = 1;  // 开启ADC中断
    ADC_CONTR &= ~ADC_CONTR_CH_MASK;                      // 清除通道选择位
    ADC_CONTR |= adc_channel_mapping[adc_ctrl.channel];  // 设置通道
    ADC_CONTR |= ADC_CONTR_START;                         // 启动转换
}

// 获取ADC转换完成的raw值，转换完成返回值，未完成返回ADC_NOT_READY
uint16_t adc_get_raw_value(adc_channel_t channel) {
    uint16_t value;

    adc_enter_critical();  // 进入ADC临界区，确保读取过程中ADC中断不会修改数据
    if (adc_ctrl.converting) {
        value = ADC_NOT_READY;
    } else {
        value = adc_raw_values[channel];
    }
    adc_exit_critical();  // 退出ADC临界区，恢复ADC中断状态
    return value;
}

// ADC 中断服务函数
void adc_Isr(void) interrupt 5 {
    if (ADC_CONTR & ADC_CONTR_FLAG) {
        // 读取ADC结果
        uint16_t adc_result = ((uint16_t)ADC_RES << 8) | ADC_RESL;

        // 保存采样值(只保留10位有效值)
        adc_ctrl.temp_values[adc_ctrl.round][adc_ctrl.channel] = adc_result & 0x03FF;

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
            EADC = 0;  // 关闭ADC中断，避免数据未处理再进中断
        } else {
            // 启动下一个通道转换
            ADC_CONTR &= ~ADC_CONTR_CH_MASK;                      // 清除通道选择位
            ADC_CONTR |= adc_channel_mapping[adc_ctrl.channel];  // 设置通道
            ADC_CONTR |= ADC_CONTR_START;                         // 启动转换
        }

        ADC_CONTR &= ~ADC_CONTR_FLAG;  // 清除中断标志位
    }
}
