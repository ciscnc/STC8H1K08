/**
 * @file Task.c
 * @brief 任务调度器源文件
 *
 * @date 2026-02-07
 */

#include "task.h"
#include "gl08_control.h"
#include "bsp_led.h"
#include "isp_trigger.h"

// 任务结构体
typedef struct {
    uint8_t Run;             // 任务状态：Run/Stop
    uint16_t TIMCount;       // 定时计数器
    uint16_t TRITime;        // 重载计数器
    void (*TaskHook)(void);  // 任务函数
} TASK_COMPONENTS;

// 任务注册表
static TASK_COMPONENTS Task_Comps[] = {
    {0, 5, 5, control_task},  // 5ms 周期，控制任务
    {0, 1000, 1000, led_task},  // 1000ms 周期，LED 翻转任务
	{0, 1, 1, isp_trigger_check},  // 1ms周期：ISP口令检测（优先级最高）
};

// 计算任务数量
static const uint8_t Tasks_Max = sizeof(Task_Comps) / sizeof(Task_Comps[0]);

/**
 * @brief 任务标记回调函数
 *
 */
void Task_Marks_Handler_Callback(void) {
    uint8_t i;
    for (i = 0; i < Tasks_Max; i++) {
        if (Task_Comps[i].TIMCount) /* If the time is not 0 */
        {
            Task_Comps[i].TIMCount--;        /* Time counter decrement */
            if (Task_Comps[i].TIMCount == 0) /* If time arrives */
            {
                /*Resume the timer value and try again */
                Task_Comps[i].TIMCount = Task_Comps[i].TRITime;
                Task_Comps[i].Run = 1; /* The task can be run */
            }
        }
    }
}

/**
 * @brief 任务处理回调函数
 *
 */
void Task_Pro_Handler_Callback(void) {
    uint8_t i;
    for (i = 0; i < Tasks_Max; i++) {
        if (Task_Comps[i].Run) /* If task can be run */
        {
            Task_Comps[i].Run = 0;    /* Flag clear 0 */
            Task_Comps[i].TaskHook(); /* Run task */
        }
    }
}
