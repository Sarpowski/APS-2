/*********************************/
/*          task.cpp              */
/*********************************/
#include <stdio.h>

#include "sys.h"
#include "rtos_api.h"

extern int SystemTick;
extern int TaskLastRun[MAX_TASK];

void ActivateTask(TTaskCall entry, int priority, char* name)
{
    int task, occupy;

    printf("ActivateTask %s\n", name);

    task = RunningTask;

    occupy = FreeTask;
    FreeTask = TaskQueue[occupy].ref;

    TaskQueue[occupy].priority = priority;
    TaskQueue[occupy].ceiling_priority = priority;
    TaskQueue[occupy].name = name;
    TaskQueue[occupy].entry = entry;
    TaskQueue[occupy].state = TASK_READY;
    TaskQueue[occupy].waiting_event = -1;

    TaskLastRun[occupy] = SystemTick;

    Schedule(occupy, INSERT_TO_TAIL);

    if (task != RunningTask)
    {
        Dispatch(task);
    }

    printf("End of ActivateTask %s\n", name);
}

void TerminateTask(void)
{
    int task;

    task = RunningTask;

    printf("TerminateTask %s\n", TaskQueue[task].name);

    RunningTask = TaskQueue[task].ref;

    TaskQueue[task].ref = FreeTask;
    FreeTask = task;

    if (RunningTask == -1)
    {
        printf("No more tasks, entering idle loop\n");
        IdleLoop();
    }

    printf("End of TerminateTask %s\n", TaskQueue[task].name);
}

// Create a task but don't activate it (POSIX-like)
int CreateTask(TTaskCall entry, int priority, char* name)
{
    int occupy;

    // Get a free slot
    if (FreeTask == -1)
    {
        printf("ERROR: No free task slots\n");
        return -1;
    }

    occupy = FreeTask;
    FreeTask = TaskQueue[occupy].ref;

    TaskQueue[occupy].priority = priority;
    TaskQueue[occupy].ceiling_priority = priority;
    TaskQueue[occupy].name = name;
    TaskQueue[occupy].entry = entry;
    TaskQueue[occupy].state = TASK_READY;
    TaskQueue[occupy].waiting_event = -1;
    TaskQueue[occupy].ref = -1;

    printf("Task %s created with priority %d\n", name, priority);

    return occupy;
}

// Suspend a task
int SuspendTask(int task_id)
{
    if (task_id < 0 || task_id >= MAX_TASK)
    {
        printf("ERROR: Invalid task ID\n");
        return -1;
    }

    TaskQueue[task_id].state = TASK_WAITING;

    if (task_id == RunningTask)
    {
        int next_task = TaskQueue[task_id].ref;
        RunningTask = next_task;
        if (next_task != -1)
        {
            Dispatch(task_id);
        }
        else
        {
            IdleLoop();
        }
    }

    printf("Task %s suspended\n", TaskQueue[task_id].name);

    return 0;
}

int ResumeTask(int task_id)
{
    if (task_id < 0 || task_id >= MAX_TASK)
    {
        printf("ERROR: Invalid task ID\n");
        return -1;
    }

    if (TaskQueue[task_id].state == TASK_SUSPENDED ||
        TaskQueue[task_id].state == TASK_WAITING ||
        TaskQueue[task_id].ref == -1)
    {
        TaskQueue[task_id].state = TASK_READY;

        Schedule(task_id, INSERT_TO_TAIL);

        if (RunningTask != -1 &&
            TaskQueue[task_id].ceiling_priority > TaskQueue[RunningTask].ceiling_priority)
        {
            int prev_running = RunningTask;
            RunningTask = task_id;
            Dispatch(prev_running);
        }
        else if (RunningTask == -1)
        {
            RunningTask = task_id;
            Dispatch(-1);
        }

        printf("Task %s resumed\n", TaskQueue[task_id].name);
        return 0;
    }

    return -1;
}

// Delay a task for a number of ticks
void DelayTask(int ticks)
{
    if (RunningTask == -1) return;

    printf("Delaying task %s for %d ticks\n", TaskQueue[RunningTask].name, ticks);

    int current_task = RunningTask;

    TaskQueue[current_task].state = TASK_WAITING;

    int wake_time = SystemTick + ticks;
    RunningTask = TaskQueue[current_task].ref;

    if (RunningTask == -1)
    {
        IdleLoop();
    }
    else
    {
        Dispatch(current_task);
    }

}

void Schedule(int task, int mode)
{
    int cur, prev;
    int priority;

    printf("Schedule %s\n", TaskQueue[task].name);

    cur = RunningTask;
    prev = -1;

    priority = TaskQueue[task].ceiling_priority;

    // RMA scheduling: higher priority tasks get scheduled first
    while (cur != -1 && TaskQueue[cur].ceiling_priority > priority)
    {
        prev = cur;
        cur = TaskQueue[cur].ref;
    }

    if (mode == INSERT_TO_TAIL)
    {
        while (cur != -1 && TaskQueue[cur].ceiling_priority == priority)
        {
            prev = cur;
            cur = TaskQueue[cur].ref;
        }
    }

    TaskQueue[task].ref = cur;

    if (prev == -1)
        RunningTask = task;
    else
        TaskQueue[prev].ref = task;

    printf("End of Schedule %s\n", TaskQueue[task].name);
}

void Dispatch(int task)
{
    printf("Dispatch\n");

    do
    {
        if (TaskQueue[RunningTask].state == TASK_READY)
        {
            TaskQueue[RunningTask].state = TASK_RUNNING;
            TaskQueue[RunningTask].entry();

            if (TaskQueue[RunningTask].state == TASK_RUNNING)
            {
                TaskQueue[RunningTask].state = TASK_READY;
            }
        }
    }
    while (RunningTask != -1 && RunningTask != task);

    printf("End of Dispatch\n");
}