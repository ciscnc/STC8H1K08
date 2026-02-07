/**
 * @file gl08_switch.h
 * @brief 旋钮档位模块头文件，实现波段旋钮和功率旋钮的档位读取
 *
 * @date 2026-02-07
 */
#ifndef __GL08_SWITCH_H__
#define __GL08_SWITCH_H__

#include "gl08_config.h"

/**
 * @brief 波段档位判定函数
 *
 * @param voltage 输入电压值（单位：mV）
 * @return 波段档位
 */
uint8_t determine_band_position(uint16_t voltage);

/**
 * @brief 功率档位判定函数
 *
 * @param voltage 输入电压值（单位：mV）
 * @return 功率档位
 */
uint8_t determine_power_position(uint16_t voltage);

#endif /* __GL08_SWITCH_H__ */
