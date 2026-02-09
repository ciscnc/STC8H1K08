/**
 * @file bsp_pwm.c
 * @brief PWM模块实现
 *
 * @date 2026-02-07
 */
#include "STC8H.h"
#include "bsp_pwm.h"

// PWM捕获数据结构体
typedef struct {
    uint16_t rise_time;  // 上升沿时间
    uint16_t fall_time;  // 下降沿时间
    uint16_t duty;       // 占空比
    uint8_t complete;    // 0: 捕获未完成，1: 捕获完成
} pwm_capture_data_t;

// PWM捕获数据存储
static data volatile pwm_capture_data_t pwm_capture_data[MAX_PWM_CHANNEL] = {0};

// 保存 PWM 捕获中断使能状态
static data uint8_t pwm_ie_backup;

// 只关 PWM1 捕获中断（CC1、CC2）
static void pwm1_ic_enter(void) {
    pwm_ie_backup = PWMA_IER;
    PWMA_IER &= ~PWM_CC12_IE;  // 只关闭 CC1 + CC2 中断
}

static void pwm1_ic_exit(void) {
    PWMA_IER = pwm_ie_backup;
}

// 只关 PWM2 捕获中断（CC3、CC4）
static void pwm2_ic_enter(void) {
    pwm_ie_backup = PWMA_IER;
    PWMA_IER &= ~PWM_CC34_IE;  // 只关闭 CC3 + CC4 中断
}

static void pwm2_ic_exit(void) {
    PWMA_IER = pwm_ie_backup;
}

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

// 动态获取输入捕获到的占空比值，捕获完成返回值，未完成返回PWM_CAPTURE_NOT_READY
uint16_t get_pwm_ic_duty(uint8_t channel) {
    uint16_t ret = PWM_CAPTURE_NOT_READY;

    if (channel == PWM1) {
        pwm1_ic_enter();  // 只关 PWM1 中断
        if (pwm_capture_data[INPUT_PWM1].complete) {
            ret = pwm_capture_data[INPUT_PWM1].duty;
        }
        pwm1_ic_exit();
    } else if (channel == PWM2) {
        pwm2_ic_enter();  // 只关 PWM2 中断
        if (pwm_capture_data[INPUT_PWM2].complete) {
            ret = pwm_capture_data[INPUT_PWM2].duty;
        }
        pwm2_ic_exit();
    }

    return ret;
}

// 开始 CC1 和 CC2 双通道捕获，同时捕获P1.0引脚(PWM1)
void pwma_ic1_start(void) {

    pwm1_ic_enter();                            // 只关 PWM1
    pwm_capture_data[INPUT_PWM1].complete = 0;  // 清零捕获完成标志
    pwm1_ic_exit();

    PWMA_CCER1 |= PWM_CC12_EN;  // 使能CC1,CC2输入捕获
    PWMA_IER |= PWM_CC12_IE;    // 使能捕获中断
    PWMA_CR1 |= 0x01;    // 确保计数器运行
}

// 开始 CC3 和 CC4 双通道捕获，同时捕获P1.4引脚(PWM2)
void pwma_ic2_start(void) {

    pwm2_ic_enter();                            // 只关 PWM2
    pwm_capture_data[INPUT_PWM2].complete = 0;  // 清零捕获完成标志
    pwm2_ic_exit();

    PWMA_CCER2 |= PWM_CC34_EN;  // 使能CC3,CC4输入捕获
    PWMA_IER |= PWM_CC34_IE;    // 使能捕获中断
    PWMA_CR1 |= 0x01;    // 确保计数器运行
}

// 停止捕获 PWM1
void pwma_ic1_stop(void) {
    PWMA_CCER1 &= ~PWM_CC12_EN;  // 关闭CC1,CC2输入捕获
    PWMA_IER &= ~PWM_CC12_IE;    // 关闭捕获中断
    // 注意：这里不停止计数器，因为可能其他功能还在使用
}

// 停止捕获 PWM2
void pwma_ic2_stop(void) {
    PWMA_CCER2 &= ~PWM_CC34_EN;  // 关闭CC3,CC4输入捕获
    PWMA_IER &= ~PWM_CC34_IE;    // 关闭捕获中断
    // 注意：这里不停止计数器，因为可能其他功能还在使用
}

// PWM 输入捕获中断服务函数
void pwm_ic_isr(void) interrupt 26 {
    // 捕获PWM1
    if (PWMA_SR1 & PWM_CC1_FLAG) {  // CC1上升沿捕获
        pwm_capture_data[INPUT_PWM1].rise_time = PWMA_CCR1;
        PWMA_SR1 &= ~PWM_CC1_FLAG;  // 清除中断标志位
    }
    if (PWMA_SR1 & PWM_CC2_FLAG) {  // CC2下降沿捕获
        pwm_capture_data[INPUT_PWM1].fall_time = PWMA_CCR2;

        // 计算高电平时间，周期不变，且PWM分频系数一样，高电平时间即为占空比
        if (pwm_capture_data[INPUT_PWM1].fall_time >= pwm_capture_data[INPUT_PWM1].rise_time) {
            pwm_capture_data[INPUT_PWM1].duty = pwm_capture_data[INPUT_PWM1].fall_time - pwm_capture_data[INPUT_PWM1].rise_time;
        } else {
            pwm_capture_data[INPUT_PWM1].duty = (0xFFFF - pwm_capture_data[INPUT_PWM1].rise_time) + pwm_capture_data[INPUT_PWM1].fall_time;
        }
        pwm_capture_data[INPUT_PWM1].complete = 1;

        // 关闭PWM1捕获中断（不停止捕获）
        PWMA_IER &= ~PWM_CC12_IE;  // 关闭 CC1 + CC2 中断

        PWMA_SR1 &= ~PWM_CC2_FLAG;  // 清标志
    }

    // 捕获PWM2
    if (PWMA_SR1 & PWM_CC3_FLAG) {  // CC3上升沿捕获
        pwm_capture_data[INPUT_PWM2].rise_time = PWMA_CCR3;
        PWMA_SR1 &= ~PWM_CC3_FLAG;
    }
    if (PWMA_SR1 & PWM_CC4_FLAG) {  // CC4下降沿捕获
        pwm_capture_data[INPUT_PWM2].fall_time = PWMA_CCR4;

        // 计算高电平时间，周期不变，且PWM分频系数一样，高电平时间即为占空比
        if (pwm_capture_data[INPUT_PWM2].fall_time >= pwm_capture_data[INPUT_PWM2].rise_time) {
            pwm_capture_data[INPUT_PWM2].duty = pwm_capture_data[INPUT_PWM2].fall_time - pwm_capture_data[INPUT_PWM2].rise_time;
        } else {
            pwm_capture_data[INPUT_PWM2].duty = (0xFFFF - pwm_capture_data[INPUT_PWM2].rise_time) + pwm_capture_data[INPUT_PWM2].fall_time;
        }
        pwm_capture_data[INPUT_PWM2].complete = 1;

        // 关闭PWM2捕获中断（不停止捕获）
        PWMA_IER &= ~PWM_CC34_IE;  // 关闭 CC3 + CC4 中断

        PWMA_SR1 &= ~PWM_CC4_FLAG;
    }
}
