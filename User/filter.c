/**
 * @file filter.c
 * @brief 滤波算法实现文件
 *
 * @date 2026-01-09
 */

#pragma NOAREGS

#include "filter.h"
#include <math.h>  // 使用C99标准的math库提供abs函数

/**
 * @brief 滤波器初始化（仅需初始值+N，极简）
 *
 * @param en 使能位
 * @param init_val 初始值（如0）
 * @param n 滤波长度N
 * @param pFilter 滤波状态结构体
 */
void ewma_filter_init(bool en, uint16_t init_val, uint16_t n, ewma_filter_t* pFilter) {
  if(!en || pFilter == NULL || n <= 0) return;
  pFilter->ave = init_val;  // 第一次采样直接赋值（符合你说的初始逻辑）
  pFilter->n = n;
  pFilter->is_first = true;
}

/**
 * @brief 重置滤波器到指定值
 * 强制将滤波器的平均值同步为指定值，并重置首轮标志。
 * 常用于信号源切换时，消除旧信号的"惯性"干扰。
 *
 * @param pFilter 滤波状态结构体
 * @param reset_val 重置后的目标值（通常为当前最新的原始采样值）
 */
void ewma_filter_reset(ewma_filter_t* pFilter, uint16_t reset_val) {
    if (pFilter == NULL) return;

    pFilter->ave = reset_val;      // 强制更新历史平均值
    pFilter->is_first = false;     // 既然已经有了明确的重置值，就不再视作"从未采样"
}

/**
 * @brief 单次指数平滑滤波器（整合限幅+死区，多类型采样值）
 *
 * @param en 使能位
 * @param sample 样本值
 * @param die 死区阈值（偏差≤die则滤波，＞die直接用采样值）
 * @param max_err 限幅阈值（偏差＞max_err则丢弃，用旧值）
 * @param pFilter 滤波状态结构体
 * @return 本次滤波输出值
 */
uint16_t ewma_filter_update(bool en, uint16_t sample, uint16_t die, uint16_t max_err,
                        ewma_filter_t* pFilter) {
  uint16_t err;
  uint16_t n;

  // 入参合法性检查
  if(!en || pFilter == NULL || pFilter->n <= 0 || max_err < die) {
    return pFilter->ave;
  }

  // 核心修复：第一次采样强制赋值，避免初始0导致限幅死循环
  if(pFilter->is_first) {
    pFilter->ave = sample;
    pFilter->is_first = false;
    return sample;
  }

  // 限幅滤波（防极端干扰）
  err = abs((int16_t)sample - (int16_t)pFilter->ave);
  if(err > max_err) {
    return pFilter->ave;  // 丢弃异常值，沿用旧平均值
  }

  // 死区判断（平衡平滑与响应）
  if(err > die) {
    // 死区外：快速响应，直接更新平均值
    pFilter->ave = sample;
  } else {
    // 死区内：加权平均滤波（核心公式，仅1个变量）
    n = pFilter->n;
    pFilter->ave = (pFilter->ave * (n - 1) + sample) / n;
  }

  return pFilter->ave;
}
