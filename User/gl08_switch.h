/*
 * GL08 旋钮档位模块头文件
 */

#ifndef __GL08_SWITCH_H__
#define __GL08_SWITCH_H__

#include "gl08_config.h"

// 函数声明
uint8_t read_band_switch(uint8_t band_switch);
uint8_t read_power_switch(void);
void scan_switches(void);

// 全局变量声明
extern volatile uint8_t band_switch_1_pos;
extern volatile uint8_t band_switch_2_pos;
extern volatile uint8_t power_switch_pos;

#endif  // __GL08_SWITCH_H__
