/*
 * GL08 双通道控制板控制逻辑实现
 * 包含调光控制、模式切换等核心控制功能
 */
#include "type_def.h"
#include "gl08_control.h"
#include "gl08_hardware.h"
#include "gl08_switch.h"
#include "bsp_adc.h"
#include "bsp_pwm.h"
#include "bsp_uart.h"
#include "filter.h"
#include "gl08_config.h"

#define DUTY_CNT_MAX PWM_FREQUENCY  // 占空比最大值
#define DUTY_CNT_MIN 0     			// 占空比最小值

// PWM滤波参数
#define PWM_FILTER_DIE 10       // 滤波死区阈值
#define PWM_FILTER_MAX_ERR 100  // 滤波限幅阈值
#define PWM_FILTER_N 4          // 滤波长度N
#define PWM_OUTPUT_THRESHOLD 5  // 输出抖动阈值

// PWM捕获超时相关宏定义
#define PWM_TIMEOUT_THRESHOLD  2    // 超时阈值（2个控制任务周期）
#define PWM_DC_LEVEL_CNT     3    // 直流电平稳定计数阈值
#define PWM_DUTY_CHANGE_THRESHOLD  10  // 占空比变化阈值

// 占空比端点锁定宏定义
#define PWM_DUTY_LOW_ENTER   10    // 低空区域进入阈值
#define PWM_DUTY_LOW_EXIT    20    // 低空区域退出阈值
#define PWM_DUTY_HIGH_ENTER  990   // 高空区域进入阈值
#define PWM_DUTY_HIGH_EXIT   980   // 高空区域退出阈值
#define PWM_ZONE_STABLE_ENTER_CNT  2  // 进入区域稳定计数阈值
#define PWM_ZONE_STABLE_EXIT_CNT   4  // 退出区域稳定计数阈值

// 输出窗口判断宏：判断输出值和当前值的差值是否超过阈值
#define OUTPUT_NEED_UPDATE(current, output, threshold) \
    ((uint16_t)((output) > (current) ? (output) - (current) : (current) - (output)) >= (threshold))

// 占空比端点状态枚举
typedef enum {
    DUTY_ZONE_NORMAL = 0,
    DUTY_ZONE_LOW_LOCK,     // 锁定 0
    DUTY_ZONE_HIGH_LOCK,    // 锁定 1000
} duty_zone_t;

// 直流检测状态枚举
typedef enum {
    DC_RES_PENDING = 0, // 待定状态，采样确认中
    DC_RES_LOW,         // 确认是持续低电平 (0% Duty)
    DC_RES_HIGH         // 确认是持续高电平 (100% Duty)
} dc_res_t;

// 直流电平滤波结构体
typedef struct {
    uint8_t temp_level;  // 临时候选电平
    uint8_t level_cnt;   // 输入电平稳定计数
} dc_filter_state_t;

// 占空比端点控制结构体
typedef struct {
    duty_zone_t zone;           // 占空比端点状态
    uint8_t stable_cnt;         // 占空比端点稳定计数
} duty_zone_ctrl_t;

// 控制通道数量枚举
typedef enum { GL08_CHANNEL1 = 0, GL08_CHANNEL2, MAX_CHANNEL } gl08_channel_t;

// 控制状态结构体
typedef struct {
    uint16_t input_value;   // PWM输入值（归一化到0-1000）
    uint16_t output_value;  // PWM输出值（归一化到0-1000）
    uint8_t band_position;  // 波段位置
    uint8_t control_mode;   // 控制模式
    uint8_t power_limit;    // 功率限制档位
    uint8_t timeout;        // PWM捕获超时计数
} control_state_t;

// Global control data
data control_state_t control_state[MAX_CHANNEL];  // 双通道，两个单独的控制结构体

// PWM捕获滤波器数组
static data ewma_filter_t pwm_filters[MAX_CHANNEL];

// 直流电平滤波器数组
static data dc_filter_state_t pwm_dc_filter[MAX_CHANNEL];

// 占空比端点控制数组
static data duty_zone_ctrl_t pwm_zone[MAX_CHANNEL];

// 上次控制模式，用于检测模式切换
static data uint8_t last_control_mode[MAX_CHANNEL];

// 内部函数声明
static uint16_t apply_power_limit(uint8_t power_limit, uint16_t value);
static uint16_t apply_band_setting(uint8_t band_position, uint16_t range);
static uint16_t apply_endpoint_lock(uint16_t duty_in, duty_zone_ctrl_t* a);
static dc_res_t dc_level_check(uint8_t current_level, dc_filter_state_t* state);

// 控制逻辑结构体初始化
void control_init(void) {
    uint8_t i;

    // 初始化PWM滤波器和控制数据
    for (i = 0; i < MAX_CHANNEL; i++) {
        ewma_filter_init(true, 0, PWM_FILTER_N, &pwm_filters[i]);

        // 初始化直流电平滤波器
        pwm_dc_filter[i].temp_level = 0;
        pwm_dc_filter[i].level_cnt = 0;

        // 初始化占空比端点控制
        pwm_zone[i].zone = DUTY_ZONE_NORMAL;
        pwm_zone[i].stable_cnt = 0;

        // 初始化控制状态
        control_state[i].input_value = 0;
        control_state[i].output_value = 0;
        control_state[i].control_mode = CONTROL_MODE_EXT;
        control_state[i].power_limit = POWER_LIMIT_100;
        control_state[i].band_position = BAND_EXT;
        control_state[i].timeout = 0;
        last_control_mode[i] = CONTROL_MODE_EXT;
    }
}

// 第一次启动转换
void first_start_conversion(void) {
    adc_start_conversion(true);  // 强制启动，确保第一次进入控制任务时有数据可用
    pwma_ic1_start();
    pwma_ic2_start();
}

// 控制任务主循环
void control_task(void) {
    uint16_t adc_raw;
    uint16_t voltage;
    uint16_t capture_raw;
    uint16_t pwm_value;
    uint16_t target_value;
    uint8_t i;
    uint8_t raw_level;
    dc_res_t res;

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

            // 判断是否捕获完成
            if (capture_raw != PWM_CAPTURE_NOT_READY) {
                // 正常捕获完成
                control_state[i].timeout = 0;  // 清除超时计数
                target_value = capture_raw;  // 直接使用捕获值
            } else {
                // 未捕获完成，进行超时处理
                control_state[i].timeout++;

                if (control_state[i].timeout >= PWM_TIMEOUT_THRESHOLD) {
                    control_state[i].timeout = PWM_TIMEOUT_THRESHOLD;

                    // 读取瞬时电平
                    if (i == GL08_CHANNEL1) {
                        raw_level = READ_PWM1_INPUT() ? 1 : 0;
                    } else {
                        raw_level = READ_PWM2_INPUT() ? 1 : 0;
                    }

                    // 调用直流电平检测
                    res = dc_level_check(raw_level, &pwm_dc_filter[i]);

                    if (res == DC_RES_HIGH) {
                        target_value = DUTY_CNT_MAX;  // 1000 (100%)
                    } else if (res == DC_RES_LOW) {
                        target_value = DUTY_CNT_MIN;  // 0 (0%)
                    } else {
                        // 待定状态，保持上次值不变
                        target_value = control_state[i].input_value;
                    }
#if UART_PRINT
                    uart_sendstr("PWM timeout, dc_level:");
                    uart_uint8(raw_level);
                    uart_print_u16("target_value:", target_value);
#endif
                } else {
                    // 未达到超时阈值，保持上次值
                    target_value = control_state[i].input_value;
                }
            }

            // 应用端点锁定
            target_value = apply_endpoint_lock(target_value, &pwm_zone[i]);

            // 检测模式切换并复位滤波器
            if (last_control_mode[i] != CONTROL_MODE_EXT) {
                ewma_filter_reset(&pwm_filters[i], target_value);
                last_control_mode[i] = CONTROL_MODE_EXT;
#if UART_PRINT
                uart_sendstr("mode switch to EXT, reset filter\r\n");
#endif
            }

            // 检测是否有显著变化
            if (IN_WINDOW(target_value, control_state[i].input_value, PWM_DUTY_CHANGE_THRESHOLD) == 0) {
                // 有显著变化，进行滤波更新
                control_state[i].input_value = ewma_filter_update(
                    true, target_value, PWM_FILTER_DIE, PWM_FILTER_MAX_ERR, &pwm_filters[i]);
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
            // 直接使用计算值（频率固定1KHz，周期=1000）
            control_state[i].input_value = pwm_value;
#if UART_PRINT
            uart_print_u16("local mode output:", pwm_value);
#endif
        }

        // 应用功率限制
        control_state[i].output_value =
            apply_power_limit(control_state[i].power_limit, control_state[i].input_value);

#if UART_PRINT
        if (i == GL08_CHANNEL1) {
            uart_print_u16("ch1 final output:", control_state[i].output_value);
        } else {
            uart_print_u16("ch2 final output:", control_state[i].output_value);
        }
#endif

        // 输出PWM，抖动小于阈值时不输出
        if (OUTPUT_NEED_UPDATE(control_state[i].output_value, control_state[i].output_value,
                               PWM_OUTPUT_THRESHOLD)) {
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
    adc_start_conversion(false);  // 非强制模式，避免重复启动
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

/**
 * @brief 端点锁定函数，防止端点抖动
 *
 * @param duty_in 输入占空比（0-1000）
 * @param a 端点控制结构体指针
 * @return 锁定后的占空比
 */
static uint16_t apply_endpoint_lock(uint16_t duty_in, duty_zone_ctrl_t* a) {
    switch (a->zone) {
    case DUTY_ZONE_NORMAL:
        if (duty_in <= PWM_DUTY_LOW_ENTER) {
            if (++a->stable_cnt >= PWM_ZONE_STABLE_ENTER_CNT) {
                a->zone = DUTY_ZONE_LOW_LOCK;
                a->stable_cnt = 0;
                return DUTY_CNT_MIN;  // 锁定为最小值
            }
        } else if (duty_in >= PWM_DUTY_HIGH_ENTER) {
            if (++a->stable_cnt >= PWM_ZONE_STABLE_ENTER_CNT) {
                a->zone = DUTY_ZONE_HIGH_LOCK;
                a->stable_cnt = 0;
                return DUTY_CNT_MAX;  // 锁定为最大值
            }
            return PWM_DUTY_HIGH_ENTER;
        } else {
            a->stable_cnt = 0;
        }
        return duty_in;

    case DUTY_ZONE_LOW_LOCK:
        if (duty_in >= PWM_DUTY_LOW_EXIT) {
            if (++a->stable_cnt >= PWM_ZONE_STABLE_EXIT_CNT) {
                a->zone = DUTY_ZONE_NORMAL;
                a->stable_cnt = 0;
                return duty_in;
            }
        } else {
            a->stable_cnt = 0;
        }
        return DUTY_CNT_MIN;  // 锁定为最小值

    case DUTY_ZONE_HIGH_LOCK:
        if (duty_in <= PWM_DUTY_HIGH_EXIT) {
            if (++a->stable_cnt >= PWM_ZONE_STABLE_EXIT_CNT) {
                a->zone = DUTY_ZONE_NORMAL;
                a->stable_cnt = 0;
                return duty_in;
            }
        } else {
            a->stable_cnt = 0;
        }
        return DUTY_CNT_MAX;  // 锁定为最大值
    }

    return duty_in;
}

/**
 * @brief 直流电平检测滤波函数
 *
 * @param current_level 当前采样电平 (0 或 1)
 * @param state 直流滤波状态结构体指针
 * @return dc_res_t 直流检测结果
 */
static dc_res_t dc_level_check(uint8_t current_level, dc_filter_state_t* state) {
    // 候选值漂移检测：如果当前采样电平跟正在观察的"候选电平"不一样，重新开始
    if (current_level != state->temp_level) {
        state->temp_level = current_level;
        state->level_cnt = 1;  // 重新计数
        return DC_RES_PENDING;  // 还没确定，让上层保持现状
    }

    // 累加一致性
    state->level_cnt++;

    // 达到阈值（确认当前电平是真实的，不是干扰）
    if (state->level_cnt >= PWM_DC_LEVEL_CNT) {
        state->level_cnt = PWM_DC_LEVEL_CNT;  // 饱和计数，防止溢出
        return (current_level == 1) ? DC_RES_HIGH : DC_RES_LOW;
    }

    // 计数未达标，继续等待
    return DC_RES_PENDING;
}
