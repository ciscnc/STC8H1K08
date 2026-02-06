/*
 * GL08 PWM 模块
 * 负责 PWM 输出控制和输入捕获
 */

#include "STC8H.h"
#include "gl08_pwm.h"
#include "uart.h"

// 全局变量，存放输入捕获的占空比值
volatile uint16_t pwm1_duty = 0;  // PWM1，端口P1.0
volatile uint16_t pwm2_duty = 0;  // PWM2，端口P1.4

// 用于计算占空比的临时变量
static uint16_t pwm1_rise_time = 0;
static uint16_t pwm1_fall_time = 0;
static uint16_t pwm2_rise_time = 0;
static uint16_t pwm2_fall_time = 0;

// PWM 初始化
void pwm_init(void) {
    pwmb_oc_init();  // PWMB输出模式初始化
    pwma_ic_init();  // PWMA输入捕获模式初始化
}

// PWM比较输出初始化
void pwmb_oc_init(void) {
    PWMB_PSCR = PWMB_PSC;  // 24分频

    PWMB_PS = 0x50;  // bit7~bit4 = 0101，高级 PWM 通道 8 输出脚选择P3.4，
                     // bit7 bit6 = 01, 高级 PWM 通道 7 输出脚选择P3.3

    PWMB_CCER2 = 0x00;  // 写 CCMRx 前必须先清零 CCxE 关闭通道
    PWMB_CCMR3 = 0x60;  // 配置PWM7为PWM模式1
    PWMB_CCMR4 = 0x60;  // 配置PWM8为PWM模式1

    PWMB_CCR7 = PWM7_DUTY;  // PWM7初始化占空比
    PWMB_CCR8 = PWM8_DUTY;  // PWM8初始化占空比

    PWMB_ARR = PWMB_PERIOD - 1;  // PWMB周期

    PWMB_CCER2 = 0x11;  // 使能PWM7、PWM8通道，高电平有效

    PWMB_ENO = 0x50;  // 使能PWM7、PWM8端口输出
    PWMB_BKR = 0x80;  // 使能主输出

    PWMB_CR1 = 0x01;  // 使能计数器
}

// PWM输入捕获初始化
void pwma_ic_init(void) {
    PWMA_PSCR = PWMA_PSC;  // 24分频，计数一个 TICK 为 1us
    PWMA_ARR = 0xFFFF;     // 最大计数周期

    PWMA_PS = 0x00;  // b5b4 = 00:PWM1P映射到P1.0；b1b0 = 00:PWM3P映射到P1.4

    PWMA_CCER1 = 0x00;   // bit0关闭CC1,bit4关闭CC2
    PWMA_CCMR1 = 0x01;   // IC1为输入模式，且映射到T11TP1上
    PWMA_CCMR1 &= 0x0F;  // 滤波设为8个时钟
    PWMA_CCMR1 |= 0x30;
    PWMA_CCMR2 = 0x02;   // IC2为输入模式，且映射到T11TP2上
    PWMA_CCMR2 &= 0x0F;  // 滤波设为8个时钟
    PWMA_CCMR2 |= 0x30;

    PWMA_CCER1 |= 0x00;  // 设置捕获极性为CC1的上升沿
    PWMA_CCER1 |= 0x20;  // 设置捕获极性为CC2的下降沿

    PWMA_CCER2 = 0x00;   // bit0关闭CC3,bit4关闭CC4
    PWMA_CCMR3 = 0x01;   // IC3为输入模式，且映射到TI3TP3上
    PWMA_CCMR3 &= 0x0F;  // 滤波设为8个时钟
    PWMA_CCMR3 |= 0x30;
    PWMA_CCMR4 = 0x02;   // IC4为输入模式，且映射到TI3TP4上
    PWMA_CCMR4 &= 0x0F;  // 滤波设为8个时钟
    PWMA_CCMR4 |= 0x30;

    PWMA_CCER2 |= 0x00;  // 设置捕获极性为CC3的上升沿
    PWMA_CCER2 |= 0x20;  // 设置捕获极性为CC4的下降沿

    PWMA_CR1 = 0x01;  // 使能计数器
}

// 动态调节PWM占空比
void set_pwm_duty(uint8_t channel, uint16_t duty) {
    if (duty > PWM_FREQUENCY) {
        duty = PWM_FREQUENCY;
    }

    switch (channel) {
    case D1:
        PWMB_CCR7 = duty;  // duty 范围 0 ~ PWMB_ARR
        break;

    case D2:
        PWMB_CCR8 = duty;  // duty 范围 0 ~ PWMB_ARR
        break;

    default:
        break;
    }
}

// 动态获取输入捕获到的占空比值
uint16_t get_pwm_ic_duty(uint8_t channel) {
    if (channel == PWM1) {
        return pwm1_duty;
    }
    return pwm2_duty;
}

// 开始 CC1 和 CC2 双通道捕获，同时捕获P1.0引脚(PWM1)
void pwma_ic1_start(void) {
    PWMA_CCER1 |= 0x11;  // 使能CC1,CC2输入捕获
    PWMA_IER |= 0x06;    // 使能捕获中断
    PWMA_CR1 |= 0x01;    // 确保计数器运行
}

// 开始 CC3 和 CC4 双通道捕获，同时捕获P1.4引脚(PWM2)
void pwma_ic2_start(void) {
    PWMA_CCER2 |= 0x11;  // 使能CC3,CC4输入捕获
    PWMA_IER |= 0x18;    // 使能捕获中断
    PWMA_CR1 |= 0x01;    // 确保计数器运行
}

// 停止捕获 PWM1
void pwma_ic1_stop(void) {
    PWMA_CCER1 &= ~0x11;  // 关闭CC1,CC2输入捕获
    PWMA_IER &= ~0x06;    // 关闭捕获中断
    // 注意：这里不停止计数器，因为可能其他功能还在使用
}

// 停止捕获 PWM2
void pwma_ic2_stop(void) {
    PWMA_CCER2 &= ~0x11;  // 关闭CC3,CC4输入捕获
    PWMA_IER &= ~0x18;    // 关闭捕获中断
    // 注意：这里不停止计数器，因为可能其他功能还在使用
}

// PWM 输入捕获中断服务函数
void pwm_ic_isr(void) interrupt 26 {
    // 捕获PWM1
    if (PWMA_SR1 & 0x02) {  // CC1上升沿捕获
        pwm1_rise_time = PWMA_CCR1;
        PWMA_SR1 &= ~0x02;  // 清除中断标志位
    }
    if (PWMA_SR1 & 0x04) {  // CC2下降沿捕获
        pwm1_fall_time = PWMA_CCR2;
        DISABLE_TIMER0();

        // 计算高电平时间，周期不变，且PWM分频系数一样，高电平时间即为占空比
        if (pwm1_fall_time >= pwm1_rise_time) {
            pwm1_duty = pwm1_fall_time - pwm1_rise_time;
        } else {
            pwm1_duty = (0xFFFF - pwm1_rise_time) + pwm1_fall_time;  // 考虑计数器溢出
        }

        PWMA_SR1 &= ~0x04;  // 清标志
    }

    // 捕获PWM2
    if (PWMA_SR1 & 0x08) {  // CC3上升沿捕获
        pwm2_rise_time = PWMA_CCR3;
        PWMA_SR1 &= ~0x08;
    }
    if (PWMA_SR1 & 0x10) {  // CC4下降沿捕获
        pwm2_fall_time = PWMA_CCR4;
        DISABLE_TIMER1();

        // 计算高电平时间，周期不变，且PWM分频系数一样，高电平时间即为占空比
        if (pwm2_fall_time >= pwm2_rise_time) {
            pwm2_duty = pwm2_fall_time - pwm2_rise_time;
        } else {
            pwm2_duty = (0xFFFF - pwm2_rise_time) + pwm2_fall_time;  // 考虑计数器溢出
        }

        PWMA_SR1 &= ~0x10;
    }
}
