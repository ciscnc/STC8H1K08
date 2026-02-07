/**
 * @file main.c
 * @brief 主程序入口，实现主循环和控制逻辑
 *
 * @date 2026-02-07
 */
#include "type_def.h"
#include "STC8H.h"
#include "gl08_config.h"
#include "gl08_hardware.h"
#include "gl08_control.h"
#include "task.h"

// Main function
int main(void) {
    EA = 0;

    EAXSFR();  // 使能访问XFR

    // System initialization
    system_init();

    // Hardware initialization
    hardware_init();

    // Control logic initialization
    control_init();

    // first start conversion
    first_start_conversion();  

    EA = 1;

    // Main loop
    while (1) {
        Task_Pro_Handler_Callback();
    }

    return 0;
}
