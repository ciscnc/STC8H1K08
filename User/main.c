/**
 * @file main.c
 * @brief 主程序入口，实现主循环和控制逻辑
 *
 * @date 2026-02-07
 */

#include "bsp_system.h"
#include "gl08_hardware.h"    // 硬件初始化
#include "gl08_control.h"     // 控制逻辑初始化
#include "task.h"             // 任务调度
#include "bsp_led.h"          // LED控制
#include "isp_trigger.h"      // ISP触发机制

// 主函数
int main(void) {

    EA = 0;          // 关闭总中断
    EAXSFR();        // 使能访问扩展寄存器

    // 系统初始化
    system_init();   // 失能硬件看门狗等系统级初始化

    // 硬件外设初始化
    hardware_init();  // GPIO、ADC、PWM、定时器、UART等硬件初始化

    // LED初始化
    led_init();

    // 控制逻辑初始化
    control_init();

    // 首次启动ADC转换和PWM输入捕获
    first_start_conversion();

    // ISP触发机制初始化
    isp_trigger_init();

    EA = 1;          // 开启总中断

    // 主循环
    while (1) {
        Task_Pro_Handler_Callback();  // 任务调度处理
    }

    return 0;
}
