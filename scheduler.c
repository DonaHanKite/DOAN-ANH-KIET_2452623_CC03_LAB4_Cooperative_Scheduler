#include "scheduler.h"
#include "main.h"

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint8_t Error_code_G = 0;
static uint8_t current_tasks_count = 0;
static uint32_t system_ticks_G = 0;

void SCH_Init(void) {
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask  = 0;
        SCH_tasks_G[i].Delay  = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe  = 0;
        SCH_tasks_G[i].TaskID = 0;
    }
    current_tasks_count = 0;
    system_ticks_G = 0;
    Error_code_G = 0;
}

void SCH_Update(void) {
    system_ticks_G++;

    if (current_tasks_count > 0 && SCH_tasks_G[0].pTask != 0) {
        if (SCH_tasks_G[0].Delay > 0) {
            SCH_tasks_G[0].Delay -= 1;
        }
        if (SCH_tasks_G[0].Delay == 0) {
            SCH_tasks_G[0].RunMe += 1;
        }
    }
}

uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD) {
    if (current_tasks_count >= SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        return SCH_MAX_TASKS;
    }

    uint8_t pos = 0;
    uint32_t sum_delay = 0;

    while (pos < current_tasks_count) {
        if (sum_delay + SCH_tasks_G[pos].Delay > DELAY) {
            break;
        }
        sum_delay += SCH_tasks_G[pos].Delay;
        pos++;
    }

    uint32_t new_delay = DELAY - sum_delay;

    if (pos < current_tasks_count) {
        SCH_tasks_G[pos].Delay -= new_delay;
    }

    for (uint8_t i = current_tasks_count; i > pos; i--) {
        SCH_tasks_G[i] = SCH_tasks_G[i - 1];
    }

    SCH_tasks_G[pos].pTask  = pFunction;
    SCH_tasks_G[pos].Delay  = new_delay;
    SCH_tasks_G[pos].Period = PERIOD;
    SCH_tasks_G[pos].RunMe  = (new_delay == 0) ? 1 : 0;
    SCH_tasks_G[pos].TaskID = pos;

    current_tasks_count++;
    return pos;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID >= current_tasks_count || SCH_tasks_G[taskID].pTask == 0) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return ERROR_SCH_CANNOT_DELETE_TASK;
    }

    if (taskID < current_tasks_count - 1) {
        SCH_tasks_G[taskID + 1].Delay += SCH_tasks_G[taskID].Delay;
    }

    for (uint8_t i = taskID; i < current_tasks_count - 1; i++) {
        SCH_tasks_G[i] = SCH_tasks_G[i + 1];
    }

    current_tasks_count--;
    SCH_tasks_G[current_tasks_count].pTask  = 0;
    SCH_tasks_G[current_tasks_count].Delay  = 0;
    SCH_tasks_G[current_tasks_count].Period = 0;
    SCH_tasks_G[current_tasks_count].RunMe  = 0;
    SCH_tasks_G[current_tasks_count].TaskID = 0;

    return 0;
}

void SCH_Dispatch_Tasks(void) {
    while (current_tasks_count > 0 && SCH_tasks_G[0].RunMe > 0) {
        void (*pTask_temp)(void) = SCH_tasks_G[0].pTask;
        uint32_t period_temp = SCH_tasks_G[0].Period;

        SCH_Delete_Task(0);
        (*pTask_temp)();

        if (period_temp > 0) {
            SCH_Add_Task(pTask_temp, period_temp, period_temp);
        }
    }

    SCH_Report_Status();
    SCH_Go_To_Sleep();
}

uint32_t get_time(void) {
    return system_ticks_G * 10;
}

void SCH_Report_Status(void) {
}

void SCH_Go_To_Sleep(void) {
    __WFI();
}
