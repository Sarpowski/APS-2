/*************************************/
/*              event.cpp              */
/*************************************/

#include "sys.h"
#include "rtos_api.h"
#include <stdio.h>

void SetEvent(int event_id, char* name)
{
    int i;

    if (event_id < 0 || event_id >= MAX_EVENT)
    {
        printf("ERROR: Invalid event ID\n");
        return;
    }

    printf("SetEvent %s\n", name);

    // Set the event status
    EventQueue[event_id].status = EVENT_SET;
    EventQueue[event_id].name = name;

    // Wake up any tasks waiting for this event
    for (i = 0; i < MAX_TASK; i++)
    {
        if (TaskQueue[i].waiting_event == event_id)
        {
            printf("Task %s woken up by event %s\n", TaskQueue[i].name, name);

            // Mark task as ready
            TaskQueue[i].state = TASK_READY;
            TaskQueue[i].waiting_event = -1;

            // Add to ready queue
            Schedule(i, INSERT_TO_TAIL);

            // Check if we need to preempt the current task
            if (RunningTask != -1 &&
                TaskQueue[i].ceiling_priority > TaskQueue[RunningTask].ceiling_priority)
            {
                int prev_running = RunningTask;
                Dispatch(prev_running);
            }
        }
    }
}

void ClearEvent(int event_id, char* name)
{
    if (event_id < 0 || event_id >= MAX_EVENT)
    {
        printf("ERROR: Invalid event ID\n");
        return;
    }

    printf("ClearEvent %s\n", name);

    // Clear the event status
    EventQueue[event_id].status = EVENT_CLEAR;
}

void WaitEvent(int event_id, char* name)
{
    if (event_id < 0 || event_id >= MAX_EVENT)
    {
        printf("ERROR: Invalid event ID\n");
        return;
    }

    printf("WaitEvent %s\n", name);

    // If the event is already set, return immediately
    if (EventQueue[event_id].status == EVENT_SET)
    {
        printf("Event %s is already set, continuing\n", name);
        return;
    }

    // Otherwise, block the task
    int current_task = RunningTask;

    // Mark the task as waiting for this event
    TaskQueue[current_task].state = TASK_WAITING;
    TaskQueue[current_task].waiting_event = event_id;

    // Remove from ready queue
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
}