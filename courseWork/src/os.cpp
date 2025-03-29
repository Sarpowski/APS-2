/******************************/
/*          os.cpp              */
/******************************/

#include <stdio.h>
#include <cstdlib>
#include "sys.h"
#include "rtos_api.h"

// External variable declarations
extern int SystemTick;
extern int TaskPeriods[MAX_TASK];
extern int TaskDeadlines[MAX_TASK];
extern int TaskLastRun[MAX_TASK];

int StartOS(TTaskCall entry, int priority, char* name)
{
    int i;

    // Initialize system state
    RunningTask = -1;
    FreeTask = 0;
    FreeResource = 0;
    FreeEvent = 0;
    SystemTick = 0;

    printf("StartOS!\n");

    // Initialize task queue
    for(i = 0; i < MAX_TASK; i++)
    {
        TaskQueue[i].ref = i + 1;
        TaskQueue[i].state = TASK_READY;
        TaskQueue[i].waiting_event = -1;
        TaskPeriods[i] = 0;       // No periodic behavior by default
        TaskDeadlines[i] = 0;     // No deadline by default
        TaskLastRun[i] = 0;       // Not run yet
    }
    TaskQueue[MAX_TASK - 1].ref = -1;

    // Initialize resource queue
    for(i = 0; i < MAX_RES; i++)
    {
        ResourceQueue[i].priority = i + 1;
        ResourceQueue[i].task = -1;
    }
    ResourceQueue[MAX_RES - 1].priority = -1;

    for(i = 0; i < MAX_EVENT; i++)
    {
        EventQueue[i].status = EVENT_CLEAR;
    }

    ActivateTask(entry, priority, name);

    return 0;
}

void ShutdownOS()
{
    printf("ShutdownOS!\n");
}
/*
void IdleLoop()
{
    // This function runs when no tasks are ready
    while(RunningTask == -1)
    {
        // Increment system tick
        SystemTick++;

        // Check if any periodic tasks need to be activated
        CheckDeadlines();

        printf("System Idle. Tick: %d\n", SystemTick);
    }
}*/


void IdleLoop()
{
    printf("DEBUG: Entered idle loop, RunningTask = %d\n", RunningTask);

    int maxTicks = 30;
    int currentTick = 0;

    while(RunningTask == -1 && currentTick < maxTicks)
    {
        SystemTick++;
        currentTick++;

        CheckDeadlines();

        printf("System Idle. Tick: %d\n", SystemTick);
    }

    if (currentTick >= maxTicks) {
        printf("DEBUG: Idle loop exited due to tick limit\n");
        printf("DEBUG: No tasks were scheduled during idle period\n");
        ShutdownOS();
        exit(1); // Force exit for debugging
    }
}

void CheckDeadlines()
{
    int i;

    for(i = 0; i < MAX_TASK; i++)
    {
        if (TaskPeriods[i] <= 0) continue;

        if ((SystemTick - TaskLastRun[i]) >= TaskPeriods[i])
        {
            if (TaskQueue[i].ref != -1 && TaskQueue[i].state != TASK_RUNNING)
            {
                TaskLastRun[i] = SystemTick;

                if (TaskQueue[i].entry != NULL)
                {
                    ActivateTask(TaskQueue[i].entry, TaskQueue[i].priority, TaskQueue[i].name);
                    printf("Periodic task %s activated at tick %d\n", TaskQueue[i].name, SystemTick);
                }
            }
        }
    }
}

// Sets the period for a task (for RMA)
void SetTaskPeriod(int task_id, int period)
{
    if (task_id >= 0 && task_id < MAX_TASK)
    {
        TaskPeriods[task_id] = period;
        printf("Task %s period set to %d\n", TaskQueue[task_id].name, period);
    }
}

// Sets the deadline for a task (for RMA)
void SetTaskDeadline(int task_id, int deadline)
{
    if (task_id >= 0 && task_id < MAX_TASK)
    {
        TaskDeadlines[task_id] = deadline;
        printf("Task %s deadline set to %d\n", TaskQueue[task_id].name, deadline);
    }
}
