/*
 * GL08 双通道控制板控制逻辑头文件
 * 包含调光控制、模式切换等核心控制功能
 */

#ifndef __GL08_CONTROL_H__
#define __GL08_CONTROL_H__

#include "gl08_config.h"

// Control logic initialization
void control_init(void);

// Input processing functions
void process_external_pwm_inputs(void);
void process_band_switch(void);
void process_power_switch(void);

// Output control functions
void update_outputs(void);
void apply_power_limit(uint8_t channel, uint16_t *value);

// Mode control functions
void set_control_mode(uint8_t gl08_channel, uint8_t mode);
uint8_t get_control_mode(uint8_t gl08_channel);

// Data processing functions
uint16_t calculate_output_value(uint8_t channel);
uint16_t apply_band_setting(uint8_t band_switch, uint16_t input_value);

// Input collecting function
void collect_inputs(void);

// External variable declarations
extern volatile uint8_t band_switch_1_pos;
extern volatile uint8_t band_switch_2_pos;
extern volatile uint8_t power_switch_pos;

extern volatile uint16_t adc_values[3];

#endif // __GL08_CONTROL_H__
