/**
 * @file gl08_switch.c
 * @brief 旋钮档位模块实现
 *
 * @date 2026-02-07
 */
#include "gl08_switch.h"

// 波段旋钮档位判定函数，输入电压，返回档位
uint8_t determine_band_position(uint16_t voltage) {
    if (IN_WINDOW(voltage, BAND_SWITCH_EXT_VOLGATE, BAND_SWITCH_ERR_VOLGATE)) {
        return BAND_EXT;
    } else if (IN_WINDOW(voltage, BAND_SWITCH_1_VOLGATE, BAND_SWITCH_ERR_VOLGATE)) {
        return BAND_0;
    } else if (IN_WINDOW(voltage, BAND_SWITCH_2_VOLGATE, BAND_SWITCH_ERR_VOLGATE)) {
        return BAND_25;
    } else if (IN_WINDOW(voltage, BAND_SWITCH_3_VOLGATE, BAND_SWITCH_ERR_VOLGATE)) {
        return BAND_50;
    } else if (IN_WINDOW(voltage, BAND_SWITCH_4_VOLGATE, BAND_SWITCH_ERR_VOLGATE)) {
        return BAND_75;
    } else if (IN_WINDOW(voltage, BAND_SWITCH_5_VOLGATE, BAND_SWITCH_ERR_VOLGATE)) {
        return BAND_100;
    }
    return BAND_NONE;  // 默认返回无波段
}

// 功率旋钮档位判定函数，输入电压，返回档位
uint8_t determine_power_position(uint16_t voltage) {
    if (IN_WINDOW(voltage, POWER_SWITCH_1_VOLGATE, POWER_SWITCH_ERR_VOLGATE)) {
        return POWER_LIMIT_67;
    } else if (IN_WINDOW(voltage, POWER_SWITCH_2_VOLGATE, POWER_SWITCH_ERR_VOLGATE)) {
        return POWER_LIMIT_83;
    } else if (IN_WINDOW(voltage, POWER_SWITCH_3_VOLGATE, POWER_SWITCH_ERR_VOLGATE)) {
        return POWER_LIMIT_100;
    }
    return POWER_LIMIT_NONE;  // 默认返回无功率档位
}
