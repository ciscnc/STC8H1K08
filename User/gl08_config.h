/*
 * GL08 双通道控制板硬件配置头文件
 * 系统配置，引脚定义和常量
 */

#ifndef __GL08_CONFIG_H__
#define __GL08_CONFIG_H__

#include "STC8H.h"
#include "type_def.h"

// 系统配置
#define FOSC 24000000UL  // 系统时钟频率 24MHz
#define BAUD 115200        // 串口波特率
#define BRT (65536 - (FOSC / BAUD + 2) / 4)  // 波特率定时器重载值

#define UART_PRINT 1  // 串口调试打印，1使能串口打印

// 窗口判断宏：判断value与target的差值是否在window范围内
#define IN_WINDOW(value, target, window) \
    ((uint16_t)((value) > (target) ? (value) - (target) : (target) - (value)) <= (window))

// PWM输出引脚操作宏
#define PWM_OUT1_SET() (P3 |= (1 << 3))
#define PWM_OUT1_CLR() (P3 &= ~(1 << 3))
#define PWM_OUT2_SET() (P3 |= (1 << 4))
#define PWM_OUT2_CLR() (P3 &= ~(1 << 4))

// 读取PWM输入引脚电平宏
#define READ_PWM1_INPUT() (P1 & (1 << 0))
#define READ_PWM2_INPUT() (P1 & (1 << 4))

// 波段旋钮档位定义
#define BAND_NONE 0   // 无效波段
#define BAND_EXT 1    // 外部控制模式
#define BAND_0 2      // 0% 输出
#define BAND_25 3     // 25% 输出
#define BAND_50 4     // 50% 输出
#define BAND_75 5     // 75% 输出
#define BAND_100 6    // 100% 输出

// 功率限制档位定义
#define POWER_LIMIT_NONE 0  // 无效功率档位
#define POWER_LIMIT_67 1    // 66.7% 功率限制
#define POWER_LIMIT_83 2    // 83.3% 功率限制
#define POWER_LIMIT_100 3   // 100% 功率限制

// 控制模式定义
#define CONTROL_MODE_LOCAL 0  // 本地控制模式
#define CONTROL_MODE_EXT 1    // 外部面板控制模式

// 波段旋钮档位电压值（用于旋钮档位判断）
#define BAND_SWITCH_EXT_VOLGATE 0    // EXT档电压值：0V = 0mv
#define BAND_SWITCH_1_VOLGATE 1000   // 1档电压值：1V = 1000mv，0%
#define BAND_SWITCH_2_VOLGATE 1830   // 2档电压值：1.83V = 1830mv，25%
#define BAND_SWITCH_3_VOLGATE 2970   // 3档电压值：2.97V = 2970mv，50%
#define BAND_SWITCH_4_VOLGATE 4050   // 4档电压值：4.05V = 4050mv，75%
#define BAND_SWITCH_5_VOLGATE 5000   // 5档电压值：5V = 5000mv，100%

// 功率旋钮档位电压值（用于功率档位判断）
#define POWER_SWITCH_1_VOLGATE 0    // 1档电压值：0V = 0mv，66.7%
#define POWER_SWITCH_2_VOLGATE 3000  // 2档电压值：3V = 3000mv，83.3%
#define POWER_SWITCH_3_VOLGATE 4600  // 3档电压值：4.6V = 4600mv，100%

// 波段档位电压误差值
#define BAND_SWITCH_ERR_VOLGATE  200  // 波段档位允许误差：200mv
#define POWER_SWITCH_ERR_VOLGATE 200  // 功率档位允许误差：200mv

// ADC通道映射（波段旋钮和功率旋钮对应的ADC通道）
#define ADC_chanel_10 0x0A  // ADC10，引脚P3.2，采样功率旋钮档位
#define ADC_chanel_13 0x0D  // ADC13，引脚P3.5，采样波段旋钮1档位
#define ADC_chanel_14 0x0E  // ADC14，引脚P3.6，采样波段旋钮2档位

#define BAND_SWITCH_1 ADC_chanel_13  // 波段旋钮1映射到ADC13
#define BAND_SWITCH_2 ADC_chanel_14  // 波段旋钮2映射到ADC14
#define POWER_SWITCH ADC_chanel_10   // 功率旋钮映射到ADC10

#endif  // __GL08_CONFIG_H__
