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

    EventQueue[event_id].status = EVENT_SET;
    EventQueue[event_id].name = name;

    for (i = 0; i < MAX_TASK; i++)
    {
        if (TaskQueue[i].waiting_event == event_id)
        {
            printf("Task %s woken up by event %s\n", TaskQueue[i].name, name);

            TaskQueue[i].state = TASK_READY;
            TaskQueue[i].waiting_event = -1;

            Schedule(i, INSERT_TO_TAIL);

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

    if (EventQueue[event_id].status == EVENT_SET)
    {
        printf("Event %s is already set, continuing\n", name);
        return;
    }

    int current_task = RunningTask;

    TaskQueue[current_task].state = TASK_WAITING;
    TaskQueue[current_task].waiting_event = event_id;
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