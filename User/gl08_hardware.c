/**
 * @file gl08_hardware.c
 * @brief 硬件驱动实现
 *
 * @date 2026-02-07
 */
#include "gl08_hardware.h"
#include "bsp_gpio.h"
#include "bsp_adc.h"
#include "bsp_pwm.h"
#include "bsp_timer.h"
#include "bsp_uart.h"

// 硬件初始化函数
void hardware_init(void) {
    gpio_init();
    adc_init();
    pwm_init();
    timer_init();
    uart_init();
}
