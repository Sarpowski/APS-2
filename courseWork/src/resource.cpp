/*************************************/
/*             resource.cpp            */
/*************************************/

#include "sys.h"
#include "rtos_api.h"
#include <stdio.h>

void GetResource(int priority, char* name)
{
    int free_occupy;

    printf("GetResource %s\n", name);

    // Get a free slot from the resource queue
    free_occupy = FreeResource;
    FreeResource = ResourceQueue[FreeResource].priority;

    // Mark the resource as owned by the running task
    ResourceQueue[free_occupy].priority = priority;
    ResourceQueue[free_occupy].task = RunningTask;
    ResourceQueue[free_occupy].name = name;

    // Implement priority ceiling protocol
    if (TaskQueue[RunningTask].ceiling_priority < priority)
    {
        TaskQueue[RunningTask].ceiling_priority = priority;
        printf("Priority ceiling raised to %d for task %s\n",
               priority, TaskQueue[RunningTask].name);
    }
}

void ReleaseResource(int priority, char* name)
{
    int i, ResourceIndex;

    printf("ReleaseResource %s\n", name);

    // Check if this is the highest priority resource held by the task
    if (TaskQueue[RunningTask].ceiling_priority == priority)
    {
        int res_priority, task_priority;
        int our_task;

        our_task = RunningTask;
        task_priority = TaskQueue[RunningTask].priority;

        // Find the resource being released
        for (i = 0; i < MAX_RES; i++)
        {
            if (ResourceQueue[i].task != RunningTask) continue;

            res_priority = ResourceQueue[i].priority;

            if (res_priority == priority && ResourceQueue[i].name == name)
                ResourceIndex = i;
            else
            if (res_priority > task_priority)
                task_priority = res_priority;
        }

        // Update the ceiling priority after release
        TaskQueue[RunningTask].ceiling_priority = task_priority;

        // Handle preemption due to priority change
        RunningTask = TaskQueue[RunningTask].ref;
        Schedule(our_task, INSERT_TO_HEAD);

        // Free the resource
        ResourceQueue[ResourceIndex].priority = FreeResource;
        ResourceQueue[ResourceIndex].task = -1;
        FreeResource = ResourceIndex;

        // If the task is preempted due to reduced priority
        if (our_task != RunningTask && RunningTask != -1)
        {
            Dispatch(our_task);
        }
    }
    else
    {
        // Find the specific resource to release
        ResourceIndex = 0;
        while (ResourceQueue[ResourceIndex].task != RunningTask ||
               ResourceQueue[ResourceIndex].priority != priority ||
               ResourceQueue[ResourceIndex].name != name)
        {
            ResourceIndex++;
        }

        // Free the resource
        ResourceQueue[ResourceIndex].priority = FreeResource;
        ResourceQueue[ResourceIndex].task = -1;
        FreeResource = ResourceIndex;
    }
}