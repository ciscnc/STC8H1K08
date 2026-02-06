/*
 * GL08 旋钮档位模块
 * 负责波段旋钮和功率旋钮的档位读取和判断
 */

#include "STC8H.h"
#include "gl08_switch.h"
#include "gl08_adc.h"
#include "uart.h"

// 全局变量
volatile uint8_t band_switch_1_pos = 0;  // Band switch 1 position
volatile uint8_t band_switch_2_pos = 0;  // Band switch 2 position
volatile uint8_t power_switch_pos = 0;   // Power switch position

// 读取波段旋钮档位
uint8_t read_band_switch(uint8_t band_switch) {
    uint8_t switch_state = 0;
    uint16_t adc_result = 0;
    uint16_t voltage_result = 0;                  // 转换为电压的结果单位为：mv
    adc_result = adc_values[band_switch];         // 读取存放的 ADC 采样值
    voltage_result = adc_to_voltage(adc_result);  // 将采样值转换为电压值（mv）

    // 根据电压值判断档位
    if (voltage_result <= (BAND_SWITCH_EXT_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE)) {
        switch_state = BAND_EXT;
    } else if (voltage_result >= (BAND_SWITCH_1_VOLGATE - OLLOW_VOLGATE_DIFFERENCE_VALUE) &&
               voltage_result <= (BAND_SWITCH_1_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE)) {
        switch_state = BAND_0;
    } else if (voltage_result >= (BAND_SWITCH_2_VOLGATE - OLLOW_VOLGATE_DIFFERENCE_VALUE) &&
               voltage_result <= (BAND_SWITCH_2_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE)) {
        switch_state = BAND_25;
    } else if (voltage_result >= (BAND_SWITCH_3_VOLGATE - OLLOW_VOLGATE_DIFFERENCE_VALUE) &&
               voltage_result <= (BAND_SWITCH_3_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE)) {
        switch_state = BAND_50;
    } else if (voltage_result >= (BAND_SWITCH_4_VOLGATE - OLLOW_VOLGATE_DIFFERENCE_VALUE) &&
               voltage_result <= (BAND_SWITCH_4_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE)) {
        switch_state = BAND_75;
    } else if (voltage_result >= (BAND_SWITCH_5_VOLGATE - OLLOW_VOLGATE_DIFFERENCE_VALUE) &&
               voltage_result <= BAND_SWITCH_5_VOLGATE) {
        switch_state = BAND_100;
    }

    if (band_switch == 0) {
        uart_sendstr("band1 voltage(mv):");
        uart_uint16(voltage_result);
        uart_sentEnter();
    } else {
        uart_sendstr("band2 voltage(mv):");
        uart_uint16(voltage_result);
        uart_sentEnter();
    }

    return switch_state;
}

// 读取功率旋钮档位
uint8_t read_power_switch(void) {
    uint8_t switch_state = 0;
    uint16_t adc_result = 0;
    uint16_t voltage_result = 0;                  // 转换为电压的结果单位为：mv
    adc_result = adc_values[2];                   // 读取存放的 ADC 采样值
    voltage_result = adc_to_voltage(adc_result);  // 将采样值转换为电压值（mv）

    // 根据电压值判断档位
    if (voltage_result <= (POWER_SWITCH_1_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE)) {
        switch_state = POWER_LIMIT_67;
    } else if (voltage_result >= (POWER_SWITCH_2_VOLGATE - OLLOW_VOLGATE_DIFFERENCE_VALUE) &&
               voltage_result <= (POWER_SWITCH_2_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE)) {
        switch_state = POWER_LIMIT_83;
    } else if (voltage_result >= (POWER_SWITCH_3_VOLGATE - OLLOW_VOLGATE_DIFFERENCE_VALUE) &&
               voltage_result <= POWER_SWITCH_3_VOLGATE + OLLOW_VOLGATE_DIFFERENCE_VALUE) {
        switch_state = POWER_LIMIT_100;
    }

    uart_sendstr("power voltage(mv):");
    uart_uint16(voltage_result);
    uart_sentEnter();

    uart_sentEnter();

    return switch_state;
}

// 扫描各个旋钮的档位
void scan_switches(void) {
    // Read band switch position
    band_switch_1_pos = read_band_switch(0);  // 扫描波段旋钮1（K1）的档位
    band_switch_2_pos = read_band_switch(1);  // 扫描波段旋钮2（K2）的档位

    // Read power switch position
    power_switch_pos = read_power_switch();  // 扫描功率旋钮的档位

    uart_sendstr("band1 switch pos:");
    uart_uint8(band_switch_1_pos);
    uart_sentEnter();

    uart_sendstr("band2 switch pos:");
    uart_uint8(band_switch_2_pos);
    uart_sentEnter();

    uart_sendstr("power switch pos:");
    uart_uint8(power_switch_pos);
    uart_sentEnter();

    uart_sentEnter();
}
