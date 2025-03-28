/*********************************/
/*          task.cpp              */
/*********************************/
#include <stdio.h>

#include "sys.h"
#include "rtos_api.h"

// External variable declarations
extern int SystemTick;
extern int TaskLastRun[MAX_TASK];

void ActivateTask(TTaskCall entry, int priority, char* name)
{
    int task, occupy;

    printf("ActivateTask %s\n", name);

    task = RunningTask;

    // Get a free slot from the free list
    occupy = FreeTask;
    FreeTask = TaskQueue[occupy].ref;

    // Initialize the task
    TaskQueue[occupy].priority = priority;
    TaskQueue[occupy].ceiling_priority = priority;
    TaskQueue[occupy].name = name;
    TaskQueue[occupy].entry = entry;
    TaskQueue[occupy].state = TASK_READY;
    TaskQueue[occupy].waiting_event = -1;

    // Record the activation time for RMA
    TaskLastRun[occupy] = SystemTick;

    // Schedule the task according to its priority
    Schedule(occupy, INSERT_TO_TAIL);

    // Preempt the current task if the new task has higher priority (RMA)
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

    // Move to the next task in the queue
    RunningTask = TaskQueue[task].ref;

    // Put the terminated task back in the free list
    TaskQueue[task].ref = FreeTask;
    FreeTask = task;

    // If no more tasks, enter idle loop
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

    // Initialize the task
    TaskQueue[occupy].priority = priority;
    TaskQueue[occupy].ceiling_priority = priority;
    TaskQueue[occupy].name = name;
    TaskQueue[occupy].entry = entry;
    TaskQueue[occupy].state = TASK_READY;
    TaskQueue[occupy].waiting_event = -1;
    TaskQueue[occupy].ref = -1;  // Not in the ready queue

    printf("Task %s created with priority %d\n", name, priority);

    return occupy;  // Return the task ID
}

// Suspend a task
int SuspendTask(int task_id)
{
    if (task_id < 0 || task_id >= MAX_TASK)
    {
        printf("ERROR: Invalid task ID\n");
        return -1;
    }

    // Change the task state to WAITING
    TaskQueue[task_id].state = TASK_WAITING;

    // If it's the running task, reschedule
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

// Resume a suspended task
int ResumeTask(int task_id)
{
    if (task_id < 0 || task_id >= MAX_TASK)
    {
        printf("ERROR: Invalid task ID\n");
        return -1;
    }

    // Only resume if task is WAITING
    if (TaskQueue[task_id].state == TASK_WAITING)
    {
        TaskQueue[task_id].state = TASK_READY;

        // Add back to the ready queue
        Schedule(task_id, INSERT_TO_TAIL);

        // Check if we need to preempt the current task
        if (RunningTask != -1 &&
            TaskQueue[task_id].ceiling_priority > TaskQueue[RunningTask].ceiling_priority)
        {
            int prev_running = RunningTask;
            RunningTask = task_id;
            Dispatch(prev_running);
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

    // Save the current task
    int current_task = RunningTask;

    // Change state to WAITING
    TaskQueue[current_task].state = TASK_WAITING;

    // Set up to wake after delay
    int wake_time = SystemTick + ticks;

    // Move to the next task
    RunningTask = TaskQueue[current_task].ref;

    // If no more tasks, enter idle loop
    if (RunningTask == -1)
    {
        IdleLoop();
    }
    else
    {
        Dispatch(current_task);
    }

    // Later, the OS would check if it's time to wake up this task
    // In a real implementation, we'd add this to a delay queue
}

void Schedule(int task, int mode)
{
    int cur, prev;
    int priority;

    printf("Schedule %s\n", TaskQueue[task].name);

    cur = RunningTask;
    prev = -1;

    priority = TaskQueue[task].ceiling_priority;

    // Find the position in the priority queue
    // RMA scheduling: higher priority tasks get scheduled first
    while (cur != -1 && TaskQueue[cur].ceiling_priority > priority)
    {
        prev = cur;
        cur = TaskQueue[cur].ref;
    }

    if (mode == INSERT_TO_TAIL)
    {
        // For tasks with the same priority, put new task at the end
        while (cur != -1 && TaskQueue[cur].ceiling_priority == priority)
        {
            prev = cur;
            cur = TaskQueue[cur].ref;
        }
    }

    // Insert the task into the queue
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

    // Execute the running tasks until we reach the specified task
    do
    {
        // Only execute if task is in READY state
        if (TaskQueue[RunningTask].state == TASK_READY)
        {
            TaskQueue[RunningTask].state = TASK_RUNNING;
            TaskQueue[RunningTask].entry();

            // If task is still running (not terminated or suspended)
            if (TaskQueue[RunningTask].state == TASK_RUNNING)
            {
                TaskQueue[RunningTask].state = TASK_READY;
            }
        }
    }
    while (RunningTask != -1 && RunningTask != task);

    printf("End of Dispatch\n");
}