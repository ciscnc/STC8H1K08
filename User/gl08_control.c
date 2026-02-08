/*
 * GL08 双通道控制板控制逻辑实现
 * 包含调光控制、模式切换等核心控制功能
 */
#include "type_def.h"
#include "gl08_control.h"
#include "gl08_hardware.h"
#include "gl08_switch.h"
#include "gl08_adc.h"
#include "gl08_pwm.h"
#include "filter.h"
#include "uart.h"

// PWM滤波参数
#define PWM_FILTER_DIE      10   // 滤波死区阈值
#define PWM_FILTER_MAX_ERR   100  // 滤波限幅阈值
#define PWM_FILTER_N         4    // 滤波长度N
#define PWM_OUTPUT_THRESHOLD  5    // 输出抖动阈值

// 输出窗口判断宏：判断输出值和当前值的差值是否超过阈值
#define OUTPUT_NEED_UPDATE(current, output, threshold) \
    ((uint16_t)((output) > (current) ? (output) - (current) : (current) - (output)) >= (threshold))

// 控制通道数量枚举
typedef enum {
    GL08_CHANNEL1 = 0,
    GL08_CHANNEL2,
    MAX_CHANNEL
} gl08_channel_t;

// 控制状态结构体
typedef struct {
    uint16_t input_value;    // PWM输入值
    uint16_t output_value;   // PWM输出值
    uint8_t band_position;   // 波段位置
    uint8_t control_mode;    // 控制模式
    uint8_t power_limit;     // 功率限制档位
} control_state_t;

// Global control data
control_state_t control_state[MAX_CHANNEL];  // 双通道，两个单独的控制结构体

// PWM捕获滤波器数组
static ewma_filter_t pwm_filters[MAX_CHANNEL];

// 上次控制模式，用于检测模式切换
static uint8_t last_control_mode[MAX_CHANNEL];

// 内部函数声明
static uint16_t apply_power_limit(uint8_t power_limit, uint16_t value);
static uint16_t apply_band_setting(uint8_t band_position, uint16_t range);

// 控制逻辑结构体初始化
void control_init(void) {
    uint8_t i;

    // 初始化PWM滤波器和控制数据
    for (i = 0; i < MAX_CHANNEL; i++) {
        ewma_filter_init(true, 0, PWM_FILTER_N, &pwm_filters[i]);
        control_state[i].input_value = 0;
        control_state[i].output_value = 0;
        control_state[i].control_mode = CONTROL_MODE_EXT;
        control_state[i].power_limit = POWER_LIMIT_100;
        control_state[i].band_position = BAND_EXT;
        last_control_mode[i] = CONTROL_MODE_EXT;
    }
}

// 第一次启动转换
void first_start_conversion(void) {
    adc_start_conversion();
    pwma_ic1_start();
    pwma_ic2_start();
}

// 控制任务主循环
void control_task(void) {
    uint16_t adc_raw;
    uint16_t voltage;
    uint16_t capture_raw;
    uint16_t pwm_value;
    uint8_t i;

#if UART_PRINT
    uart_sendstr("====== control task begin ======\r\n");
#endif

    // 获取波段1 ADC值并转换为档位
    adc_raw = adc_get_raw_value(BAND_K1_ADC_CHANNEL);
    if (adc_raw != ADC_NOT_READY) {
        voltage = adc_to_voltage(adc_raw);
        control_state[GL08_CHANNEL1].band_position = determine_band_position(voltage);
#if UART_PRINT
        uart_print_u16("band1 voltage(mv):", voltage);
        uart_print_u8("band1 pos:", control_state[GL08_CHANNEL1].band_position);
#endif
    }

    // 获取波段2 ADC值并转换为档位
    adc_raw = adc_get_raw_value(BAND_K2_ADC_CHANNEL);
    if (adc_raw != ADC_NOT_READY) {
        voltage = adc_to_voltage(adc_raw);
        control_state[GL08_CHANNEL2].band_position = determine_band_position(voltage);
#if UART_PRINT
        uart_print_u16("band2 voltage(mv):", voltage);
        uart_print_u8("band2 pos:", control_state[GL08_CHANNEL2].band_position);
#endif
    }

    // 获取功率旋钮ADC值并转换为档位
    adc_raw = adc_get_raw_value(POWER_ADC_CHANNEL);
    if (adc_raw != ADC_NOT_READY) {
        voltage = adc_to_voltage(adc_raw);
        control_state[GL08_CHANNEL1].power_limit = determine_power_position(voltage);
        control_state[GL08_CHANNEL2].power_limit = determine_power_position(voltage);
#if UART_PRINT
        uart_print_u16("power voltage(mv):", voltage);
        uart_print_u8("power limit:", control_state[GL08_CHANNEL1].power_limit);
#endif
    }

    // 处理两个通道
    for (i = 0; i < MAX_CHANNEL; i++) {
        if (control_state[i].band_position == BAND_EXT) {
            // 外部控制模式：获取PWM捕获值
            if (i == GL08_CHANNEL1) {
                capture_raw = get_pwm_ic_duty(PWM1);
            } else {
                capture_raw = get_pwm_ic_duty(PWM2);
            }

            // 检测从本地模式切换到外部模式，复位滤波器
            if (last_control_mode[i] != CONTROL_MODE_EXT && capture_raw != PWM_CAPTURE_NOT_READY) {
                ewma_filter_reset(&pwm_filters[i], capture_raw);
                last_control_mode[i] = CONTROL_MODE_EXT;
#if UART_PRINT
                uart_sendstr("mode switch to EXT, reset filter\r\n");
#endif
            }

            if (capture_raw != PWM_CAPTURE_NOT_READY) {
                // 滤波处理
                control_state[i].input_value = ewma_filter_update(true, capture_raw, PWM_FILTER_DIE,
                                                              PWM_FILTER_MAX_ERR, &pwm_filters[i]);
#if UART_PRINT
                if (i == GL08_CHANNEL1) {
                    uart_print_u16("pwm1 capture:", capture_raw);
                } else {
                    uart_print_u16("pwm2 capture:", capture_raw);
                }
                uart_print_u16("after filter:", control_state[i].input_value);
#endif
            }
        } else {
            // 本地控制模式：根据波段位置计算输出值
            last_control_mode[i] = CONTROL_MODE_LOCAL;
            pwm_value = apply_band_setting(control_state[i].band_position, PWM_FREQUENCY);
            control_state[i].input_value = pwm_value;
#if UART_PRINT
            uart_print_u16("local mode output:", pwm_value);
#endif
        }

        // 应用功率限制
        control_state[i].output_value = apply_power_limit(control_state[i].power_limit, control_state[i].input_value);
#if UART_PRINT
        if (i == GL08_CHANNEL1) {
            uart_print_u16("ch1 final output:", control_state[i].output_value);
        } else {
            uart_print_u16("ch2 final output:", control_state[i].output_value);
        }
#endif

        // 输出PWM，抖动小于阈值时不输出
        if (OUTPUT_NEED_UPDATE(control_state[i].input_value, control_state[i].output_value, PWM_OUTPUT_THRESHOLD)) {
            if (i == GL08_CHANNEL1) {
                set_pwm_duty(D1, control_state[i].output_value);
            } else {
                set_pwm_duty(D2, control_state[i].output_value);
            }
        }
    }

#if UART_PRINT
    uart_sendstr("====== control task end ======\r\n");
	uart_sentEnter();
#endif

    // 重新启动ADC转换和PWM捕获
    adc_start_conversion();
    pwma_ic1_start();
    pwma_ic2_start();
}

// 应用功率限制，根据功率档位计算功率限制后的值
static uint16_t apply_power_limit(uint8_t power_limit, uint16_t value) {
    switch (power_limit) {
    case POWER_LIMIT_67:
        return (uint16_t)((uint32_t)value * 667 / 1000);  // 66.7%

    case POWER_LIMIT_83:
        return (uint16_t)((uint32_t)value * 833 / 1000);  // 83.3%

    case POWER_LIMIT_100:
    default:
        return value;  // 100%，无功率限制
    }
}

// 应用波段设置，根据波段位置和量程计算输出值
static uint16_t apply_band_setting(uint8_t band_position, uint16_t range) {
    switch (band_position) {
    case BAND_0:
        return range * 0;  // 0%

    case BAND_25:
        return range >> 2;  // 25% (除以4)

    case BAND_50:
        return range >> 1;  // 50% (除以2)

    case BAND_75:
        return (range * 3) >> 2;  // 75% (乘以3除以4)

    case BAND_100:
        return range;  // 100%

    case BAND_EXT:
    default:
        return 0;  // 外部档位或未知档位
    }
}
