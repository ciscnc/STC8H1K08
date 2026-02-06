/*
 * GL08 Dual Channel Control Board Main Program
 * MCU: STC8H1K08-36I-TSSOP20
 * Function: 2-channel independent dimming control
 * Author: Lighting Control Software Team
 * Date: 2025-06-19
 */

#include "type_def.h"
#include "STC8H.h"
#include "gl08_config.h"
#include "gl08_hardware.h"
#include "gl08_control.h"
#include "uart.h"

// Main function
int main(void) {
    uint16_t test2 = 0;
    EA = 0;

    EAXSFR();  // 使能访问XFR

    // System initialization
    system_init();

    // Hardware initialization
    hardware_init();

    // uart initialization
    uart_init();

    // Control logic initialization
    control_init();

    EA = 1;

    uart_sendstr("System initialize complete.\r\n");

    // Main loop
    while (1) {
        // 串口调试，可在gl08_config.h中通过宏 UART_PRINT 开启或关闭
        uart_sendstr("====== begin scan switchs ======\r\n");
        uart_sendstr("execute count:");
        uart_uint16(test2++);
        uart_sentEnter();
        uart_sentEnter();

        // Collect switch inputs
        collect_inputs();

        // Scan switch status
        scan_switches();

        // Process band switch
        process_band_switch();

        // Process external PWM inputs
        process_external_pwm_inputs();

        // Process power switch
        process_power_switch();

        // Update output control
        update_outputs();
    }

    return 0;
}
