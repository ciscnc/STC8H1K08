/**
 * @file baremetal_sem.c
 * @brief 通用裸机二值信号量模块实现
 * @note 无任何硬件相关代码，临界区函数通过注入实现松耦合
 * @date 2026-02-09
 */
#include "baremetal_sem.h"

/**
 * @brief 初始化二值信号量（绑定临界区函数）
 */
SemStatus_t sem_binary_init(BinarySem_t *sem, SemCriticalEnter_t enter, SemCriticalExit_t exit,
                            uint8_t init_value) {
    // 参数合法性检查
    if (sem == NULL || enter == NULL || exit == NULL) {
        return SEM_ERROR;
    }

    // 绑定临界区函数（松耦合核心）
    sem->enter_critical = enter;
    sem->exit_critical = exit;
    // 初始化信号量值（仅支持0/1）
    sem->value = (init_value == 0) ? 0 : 1;

    return SEM_OK;
}

/**
 * @brief 获取二值信号量（非阻塞）
 * @note 临界区仅操作信号量值，耗时几十纳秒，几乎不影响中断实时性
 */
SemStatus_t sem_binary_take(BinarySem_t *sem) {
    SemStatus_t ret = SEM_ERROR;

    // 参数检查：未初始化直接返回错误
    if (sem == NULL || sem->enter_critical == NULL || sem->exit_critical == NULL) {
        return SEM_ERROR;
    }

    // 进入临界区（注入的硬件相关函数）
    sem->enter_critical();

    // 仅操作信号量值（核心逻辑，极简）
    if (sem->value == 1) {
        sem->value = 0;
        ret = SEM_OK;
    } else {
        ret = SEM_BUSY;
    }

    // 退出临界区（注入的硬件相关函数）
    sem->exit_critical();

    return ret;
}

/**
 * @brief 释放二值信号量
 */
SemStatus_t sem_binary_give(BinarySem_t *sem) {
    // 参数检查
    if (sem == NULL || sem->enter_critical == NULL || sem->exit_critical == NULL) {
        return SEM_ERROR;
    }

    // 进入临界区
    sem->enter_critical();

    // 释放信号量（强制置1）
    sem->value = 1;

    // 退出临界区
    sem->exit_critical();

    return SEM_OK;
}

/**
 * @brief 获取当前信号量值
 */
uint8_t sem_binary_get_value(BinarySem_t *sem) {
    uint8_t val = 0xFF;

    if (sem == NULL || sem->enter_critical == NULL || sem->exit_critical == NULL) {
        return val;
    }

    sem->enter_critical();
    val = sem->value;
    sem->exit_critical();

    return val;
}
