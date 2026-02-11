# BUG 记录

## BUG-001: 任务调度器 - led_task不执行问题

### 日期
2026-02-11

### 描述
当任务表按特定顺序注册时，`led_task`无法被执行。

### 复现条件
任务表顺序如下时出现问题：
```c
static TASK_COMPONENTS Task_Comps[] = {
    {0, 1, 1, isp_trigger_check},   // 索引0：1ms周期
    {0, 5, 5, control_task},        // 索引1：5ms周期
    {0, 1000, 1000, led_task},      // 索引2：1000ms周期
};
```

### 现象
- 定时器中断正常，`Task_Marks_Handler_Callback`被1ms定时器正常调用
- `led_task`的`Run`标志被正确置位（通过调试代码验证）
- `Task_Pro_Handler_Callback`正常遍历任务表
- 但`led_task`函数从未被调用
- `led_task`中的`uart_sendstr("led_task called\r\n")`从未打印
- LED不闪烁

### 正常工作的配置

#### 配置1：调整任务表顺序
```c
static TASK_COMPONENTS Task_Comps[] = {
    {0, 5, 5, control_task},        // 索引0
    {0, 1000, 1000, led_task},      // 索引1
    {0, 1, 1, isp_trigger_check},   // 索引2
};
```
**状态：恢复正常**

#### 配置2：增大control_task周期
```c
{0, 5, 5, control_task},        // 改为
{0, 100, 100, control_task},     // 100ms周期
```
**状态：恢复正常**

### 已尝试的解决方案
1. **添加`#pragma NOOVERLAY`** - 无效
   - 在`led_task`、`control_task`、`Task_Pro_Handler_Callback`上添加
   - 预防C51编译器`?C?ICALL`函数指针调用库的BUG
   - 结果：问题依旧

2. **修改control_task周期为100ms** - 有效
   - 但这不是根本解决方案

3. **调整任务表顺序** - 有效
   - `control_task`放第一位
   - 但这不是根本解决方案

### 任务调度器代码分析

#### Task_Marks_Handler_Callback（定时器中断调用）
```c
void Task_Marks_Handler_Callback(void) {
    uint8_t i;
    for (i = 0; i < Tasks_Max; i++) {
        if (Task_Comps[i].TIMCount) /* If the time is not 0 */
        {
            Task_Comps[i].TIMCount--;
            if (Task_Comps[i].TIMCount == 0) /* If time arrives */
            {
                Task_Comps[i].TIMCount = Task_Comps[i].TRITime;
                Task_Comps[i].Run = 1; /* The task can be run */
            }
        }
    }
}
```

#### Task_Pro_Handler_Callback（主循环调用）
```c
void Task_Pro_Handler_Callback(void) {
    uint8_t i;
    for (i = 0; i < Tasks_Max; i++) {
        if (Task_Comps[i].Run) /* If task can be run */
        {
            Task_Comps[i].Run = 0;
            Task_Comps[i].TaskHook(); /* Run task */
        }
    }
}
```

### 调试信息
在主循环中添加调试代码：
```c
static uint16_t debug_cnt = 0;
if (++debug_cnt % 100 == 0) {
    uart_sendstr("loop\r\n");
}
```

**输出结果：**
- 大量`loop`打印
- `control_task`每5ms执行一次（打印正常）
- `led_task called`从未出现
- 即使等待10秒以上，`led_task`也不执行

### 函数地址信息
- `led_task`: 0x0026
- `control_task`: 0x00D6
- `isp_trigger_check`: 0x13BF

### 可能的根本原因
1. **C51编译器问题**
   - 函数指针调用时栈帧优化错误
   - 某些函数地址组合导致问题

2. **内存对齐或布局问题**
   - 任务表顺序影响代码段布局
   - 某些组合导致访问异常

3. **控制任务执行时间影响**
   - `control_task`每5ms执行，可能占用较多时间
   - 虽然调度器是顺序遍历，但某些边界条件未考虑到

### 当前状态
**临时解决**：使用能正常工作的任务表顺序
**状态**：OPEN（根本原因未找到）

### 后续建议
1. 使用Keil反汇编工具对比不同配置下的代码
2. 尝试使用直接函数调用而非函数指针
3. 优化`control_task`的执行效率
4. 使用示波器或逻辑分析仪验证时序
