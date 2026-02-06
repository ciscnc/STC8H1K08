/*
 * GL08 Timer 模块
 * 负责定时器初始化和中断处理
 */

#include "STC8H.h"
#include "gl08_timer.h"
#include "gl08_pwm.h"

// Timer 初始化
void timer_init(void) {
    // 配置 Timer0 and Timer1 为 2ms 触发溢出中断，用于检测 PWM 0% 和 100% 的输入情况
    TMOD = 0x00;  // Timer0 and Timer1 both works in mode 0
    TH0 = TIMER0_RELOAD_H;
    TL0 = TIMER0_RELOAD_L;

    // Enable Timer0 interrupt
    ET0 = 1;

    // Start Timer0
    TR0 = 1;

    TL1 = TIMER1_RELOAD_H;
    TH1 = TIMER1_RELOAD_L;

    // Enable Timer1 interrupt
    ET1 = 1;

    // Start Timer1
    TR1 = 1;
}

// Timer0 中断服务函数
void timer0_isr(void) interrupt 1 {
    TF0 = 0;  // 清除定时器0溢出中断标志

    // 在中断中关闭定时器0
    DISABLE_TIMER0();

    if (READ_PWM1_INPUT()) {  // 检测PWM1输入引脚P1.0电平值
        pwm1_duty = 1000;
    } else {
        pwm1_duty = 0;
    }
}

// Timer1 中断服务函数
void Timer1_ISR(void) interrupt 3 {
    TF1 = 0;  // 清除定时器1溢出中断标志

    // 在中断中关闭定时器1
    DISABLE_TIMER1();

    if (READ_PWM2_INPUT()) {  // 检测PWM2输入引脚P1.4电平值
        pwm2_duty = 1000;
    } else {
        pwm2_duty = 0;
    }
}
