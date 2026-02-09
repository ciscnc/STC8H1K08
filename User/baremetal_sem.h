/**
 * @file baremetal_sem.h
 * @brief 通用裸机二值信号量模块（松耦合、易移植）
 * @note 临界区函数通过注入方式绑定，支持任意MCU移植（STC8H/STM32/51/ARM）
 * @date 2026-02-09
 */
#ifndef BAREMETAL_SEM_H
#define BAREMETAL_SEM_H

#include "type_def.h"

// 前向声明：避免头文件循环依赖
typedef struct BinarySem BinarySem_t;

/**
 * @brief 临界区函数类型定义（松耦合核心）
 * @note 注入的函数需实现：保存中断状态→关中断 / 恢复中断状态
 */
typedef void (*SemCriticalEnter_t)(void);  // 进入临界区函数类型
typedef void (*SemCriticalExit_t)(void);   // 退出临界区函数类型

/**
 * @brief 二值信号量结构体（实例化时绑定临界区函数）
 */
struct BinarySem {
    uint8_t value;                      // 信号量值：1-可用，0-被占用
    SemCriticalEnter_t enter_critical;  // 注入的进入临界区函数
    SemCriticalExit_t exit_critical;    // 注入的退出临界区函数
};

/**
 * @brief 信号量操作返回值
 */
typedef enum {
    SEM_OK = 0,       // 操作成功
    SEM_BUSY = 1,     // 信号量被占用（take失败）
    SEM_ERROR = 0xFF  // 操作错误（未绑定临界区函数）
} SemStatus_t;

/**
 * @brief 初始化二值信号量（绑定临界区函数）
 * @param sem 信号量实例指针
 * @param enter 注入的进入临界区函数
 * @param exit 注入的退出临界区函数
 * @param init_value 初始值（1-可用，0-被占用）
 * @return SEM_OK-成功，SEM_ERROR-参数错误
 */
SemStatus_t sem_binary_init(BinarySem_t *sem, SemCriticalEnter_t enter, SemCriticalExit_t exit,
                            uint8_t init_value);

/**
 * @brief 获取二值信号量（非阻塞）
 * @param sem 信号量实例指针
 * @return SEM_OK-获取成功，SEM_BUSY-被占用，SEM_ERROR-未初始化
 */
SemStatus_t sem_binary_take(BinarySem_t *sem);

/**
 * @brief 释放二值信号量
 * @param sem 信号量实例指针
 * @return SEM_OK-释放成功，SEM_ERROR-未初始化
 */
SemStatus_t sem_binary_give(BinarySem_t *sem);

/**
 * @brief 获取当前信号量值
 * @param sem 信号量实例指针
 * @return 1-可用，0-被占用，0xFF-未初始化
 */
uint8_t sem_binary_get_value(BinarySem_t *sem);

#endif  // BAREMETAL_SEM_H
