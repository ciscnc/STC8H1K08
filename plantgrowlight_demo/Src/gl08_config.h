/*
 * GL08 双通道控制板硬件配置头文件
 * 系统配置，引脚定义和常量
 */

#ifndef __GL08_CONFIG_H__
#define __GL08_CONFIG_H__

#include "STC8H.h"
#include "type_def.h"
#include "uart.h"
#include "delay.h"


// 系统配置
#define FOSC            24000000UL   // 系统时钟频率
#define BAUD            9600        // 串口波特率
#define BRT             (65536 - (FOSC / BAUD+2) / 4)

#define UART_PRINT 1  //串口调试打印，1使能串口打印

// GL08 双通道
#define GL08_CH1 1    // 通道1，对应链路为波段旋钮1（K1）、PWM1、D1
#define GL08_CH2 2    // 通道2，对应链路为波段旋钮2（K2）、PWM2、D2

// Timer 配置
#define TIMER0_RELOAD_H ((65536 - FOSC/12/2000) >> 8)  // Timer0 2ms 定时器
#define TIMER0_RELOAD_L ((65536 - FOSC/12/2000) & 0xFF)

#define TIMER1_RELOAD_H ((65536 - FOSC/12/2000) >> 8)  // Timer1 2ms 定时器
#define TIMER1_RELOAD_L ((65536 - FOSC/12/2000) & 0xFF)

#define TIMER2_RELOAD_H ((65536 - FOSC/12/2000) >> 8)  // Timer2 1ms 定时器
#define TIMER2_RELOAD_L ((65536 - FOSC/12/2000) & 0xFF)

//PWMB配置，用于输出周期固定，占空比变化的PWM波
#define PWMB_PSC     (24-1)     //定义 PWMB 时钟预分频系数
#define PWMB_PERIOD  1000       //定义 PWMB 周期值,(频率=FOSC/(PWMB_PSC+1)/PWMB_PERIOD=1000Hz)
#define PWM7_DUTY    500        //定义 PWM7 的占空比值,(占空比=PWMx_DUTY/PWMA_PERIOD*100%=50%)
#define PWM8_DUTY    500 
#define D1 GL08_CH1   //PWM7，端口P3.3，与原理图对应
#define D2 GL08_CH2   //PWM8，端口P3.4，与原理图对应

//PWMA配置，用于输入捕获外部PWM输入
#define PWMA_PSC     (24-1)    //定义 PWMA 时钟预分频系数
#define PWM1 GL08_CH1  //PWM1P，端口P1.0， 与原理图对应
#define PWM2 GL08_CH2  //PWM3P，端口P1.4， 与原理图对应

#define PWM_FREQUENCY   PWMB_PERIOD        // PWM frequency 1kHz


//ADC相关宏定义，用于采集2个波段旋钮和功率旋钮档位电压
#define ADC_CHANNELS    3           // Number of ADC channels
#define ADC_RESOLUTION  1024        // 10-bit ADC resolution

#define ADC_CH1 0	   // 通道1，波段旋钮K1（此宏用于区分 ADC 转换值存放位置，并非实际 ADC 通道）
#define ADC_CH2 1    // 通道2，波段旋钮K2
#define ADC_CH3 2    // 通道3，功率旋钮
	
#define CONSECUTIVE_CONV 3   // ADC 连续转换次数

#define ADC_chanel_10 0x0A  //ADC10，引脚P3.2，采样功率旋钮档位
#define ADC_chanel_13 0x0D  //ADC13，引脚P3.5，采样波段旋钮1档位
#define ADC_chanel_14 0x0E  //ADC14，引脚P3.6，采样波段旋钮2档位

#define BAND_SWITCH_1 ADC_chanel_13  //波段旋钮1映射到ADC13
#define BAND_SWITCH_2 ADC_chanel_14  //波段旋钮2映射到ADC14
#define POWER_SWITCH  ADC_chanel_10  //功率旋钮映射到ADC10

// 波段旋钮
#define K1 GL08_CH1   
#define K2 GL08_CH2

// 波段旋钮档位电压值
#define BAND_SWITCH_EXT_VOLGATE 0     //EXT档波段电压值:  0V = 0mv，   EXT
#define BAND_SWITCH_1_VOLGATE   1000  //1档波段电压值：   1V = 1000mv，0%
#define BAND_SWITCH_2_VOLGATE   1830  //2档波段电压值：1.83V = 1830mv，25%
#define BAND_SWITCH_3_VOLGATE   2970  //3档波段电压值：2.97V = 2970mv，50%
#define BAND_SWITCH_4_VOLGATE   4050  //4档波段电压值：4.05V = 4050mv，75%
#define BAND_SWITCH_5_VOLGATE   5000  //5档波段电压值：   5V = 5000mv，100%
 
// 功率旋钮档位电压值
#define POWER_SWITCH_1_VOLGATE   0     //1档波段电压值：  0V = 0mv，   66.7%   !!!现为测试，需更改
#define POWER_SWITCH_2_VOLGATE   3000  //2档波段电压值：  3V = 3000mv，83.3%
#define POWER_SWITCH_3_VOLGATE   4600  //3档波段电压值：4.6V = 4600mv，100%

#define OLLOW_VOLGATE_DIFFERENCE_VALUE  200   //允许的电压误差值：200mv

// PWM output pin operations
#define PWM_OUT1_SET()  (P3 |= (1<<3))
#define PWM_OUT1_CLR()  (P3 &= ~(1<<3))
#define PWM_OUT2_SET()  (P3 |= (1<<4))
#define PWM_OUT2_CLR()  (P3 &= ~(1<<4))

// 读取PWM输入引脚电平
#define READ_PWM1_INPUT()  (P1 &(1<<0))
#define READ_PWM2_INPUT()  (P1 &(1<<4))

// Timer0 操作，启动和关闭
#define EABLE_TIMER0()   TR0 = 1
#define DISABLE_TIMER0() TR0 = 0

// Timer1 操作，启动和关闭
#define EABLE_TIMER1()   TR1 = 1
#define DISABLE_TIMER1() TR1 = 0

// Band switch position definitions
#define BAND_EXT        1   // External control mode
#define BAND_0          2   // 0% output
#define BAND_25         3   // 25% output
#define BAND_50         4   // 50% output
#define BAND_75         5   // 75% output
#define BAND_100        6   // 100% output

// Power limit switch position definitions
#define POWER_LIMIT_67  1   // 66.7% power limit
#define POWER_LIMIT_83  2   // 83.3% power limit
#define POWER_LIMIT_100 3   // 100% power limit

// Control mode definitions
#define CONTROL_MODE_LOCAL  0   // Local control mode
#define CONTROL_MODE_EXT    1   // External panel control mode

// Data type definitions
typedef struct {
    uint16_t channel_value;    // Channel output value (0-1000)
    uint8_t control_mode;       // Control mode
    uint8_t power_limit;        // Power limit
    uint8_t band_position;      // Band switch position
} control_data_t;

#endif // __GL08_CONFIG_H__