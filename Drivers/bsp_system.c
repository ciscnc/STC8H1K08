/**
 * @file bsp_system.c
 * @brief 系统初始化模块实现
 *
 * @date 2026-02-07
 */
#include "STC8H.h"
#include "bsp_system.h"

// System 初始化
void system_init(void) {
    // 失能硬件看门狗
    WDT_CONTR = 0x00;
}
