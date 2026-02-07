/**
 * @file task.h
 * @brief 任务调度器头文件，实现简单的任务调度管理
 *
 * @date 2026-02-07
 */
#ifndef __TASK_H__
#define __TASK_H__

#include "type_def.h"

/**
 * @brief 任务标记回调函数
 */
void Task_Marks_Handler_Callback(void);

/**
 * @brief 任务处理回调函数
 */
void Task_Pro_Handler_Callback(void);

#endif /* __TASK_H__ */
