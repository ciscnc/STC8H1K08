/**
 * @file gl08_system.c
 * @brief 系统初始化模块实现
 *
 * @date 2026-02-07
 */
#include "STC8H.h"
#include "gl08_system.h"
#include "gl08_gpio.h"
#include "gl08_adc.h"
#include "gl08_pwm.h"
#include "gl08_timer.h"
#include "uart.h"

// System 初始化
void system_init(void) {
    // 失能硬件看门狗
    WDT_CONTR = 0x00;
}

// Hardware initialization
void hardware_init(void) {
    gpio_init();
    adc_init();
    pwm_init();
    timer_init();
    uart_init();
}
