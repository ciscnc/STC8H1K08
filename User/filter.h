/**
 * @file filter.h
 * @brief 滤波算法头文件
 * 
 * @date 2026-01-09
 */

#ifndef __FILTER_H__
#define __FILTER_H__

#include "type_def.h"

// 单次指数平滑滤波算法状态结构体
typedef struct {
    uint16_t ave;      // 唯一维护的变量：滤波平均值
    uint16_t n;        // 滤波长度N（可动态调整，无需重启）
    bool is_first; // 新增：标记是否第一次采样
} ewma_filter_t;

/**
 * @brief 单次指数平滑滤波算法初始化
 * 
 * @param en 是否启用滤波
 * @param init_val 初始值
 * @param n 滤波长度N（可动态调整，无需重启）
 * @param pFilter 滤波状态结构体指针
 */
void ewma_filter_init(bool en, uint16_t init_val, uint16_t n, ewma_filter_t* pFilter);

/**
 * @brief 重置滤波器
 * 
 * @param pFilter 滤波状态结构体指针
 * @param reset_val 重置值
 */
void ewma_filter_reset(ewma_filter_t* pFilter, uint16_t reset_val);

/**
 * @brief 单次指数平滑滤波算法更新
 * 
 * @param en 是否启用滤波
 * @param sample 样本值
 * @param die 死区阈值（偏差≤die则滤波，＞die直接用采样值）
 * @param max_err 限幅阈值（偏差＞max_err则丢弃，用旧值）
 * @param pFilter 滤波状态结构体指针
 * @return uint16_t 滤波结果
 */
uint16_t ewma_filter_update(bool en, uint16_t sample, uint16_t die, uint16_t max_err, ewma_filter_t* pFilter) ;

#endif /* __FILTER_H__ */
