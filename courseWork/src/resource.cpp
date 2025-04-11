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

    free_occupy = FreeResource;
    FreeResource = ResourceQueue[FreeResource].priority;

    ResourceQueue[free_occupy].priority = priority;
    ResourceQueue[free_occupy].task = RunningTask;
    ResourceQueue[free_occupy].name = name;

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

    if (TaskQueue[RunningTask].ceiling_priority == priority)
    {
        int res_priority, task_priority;
        int our_task;

        our_task = RunningTask;
        task_priority = TaskQueue[RunningTask].priority;

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

        TaskQueue[RunningTask].ceiling_priority = task_priority;

        RunningTask = TaskQueue[RunningTask].ref;
        Schedule(our_task, INSERT_TO_HEAD);

        ResourceQueue[ResourceIndex].priority = FreeResource;
        ResourceQueue[ResourceIndex].task = -1;
        FreeResource = ResourceIndex;

        if (our_task != RunningTask && RunningTask != -1)
        {
            Dispatch(our_task);
        }
    }
    else
    {
        ResourceIndex = 0;
        while (ResourceQueue[ResourceIndex].task != RunningTask ||
               ResourceQueue[ResourceIndex].priority != priority ||
               ResourceQueue[ResourceIndex].name != name)
        {
            ResourceIndex++;
        }

        ResourceQueue[ResourceIndex].priority = FreeResource;
        ResourceQueue[ResourceIndex].task = -1;
        FreeResource = ResourceIndex;
    }
}




// Initialize the semaphore system
void InitSemaphores()
{
    int i;

    // Initialize semaphore queue
    for(i = 0; i < MAX_RES; i++)
    {
        SemaphoreQueue[i].state = SEM_AVAILABLE;
        SemaphoreQueue[i].owner_task = -1;
        SemaphoreQueue[i].priority = 0;
        SemaphoreQueue[i].name = nullptr;
    }

    FreeSemaphore = 0;

    printf("Semaphore system initialized\n");
}

// Create a semaphore
int CreateSemaphore(int priority, char* name)
{
    int sem_id;

    // Find free semaphore slot
    if (FreeSemaphore >= MAX_RES)
    {
        printf("ERROR: No free semaphore slots\n");
        return -1;
    }

    sem_id = FreeSemaphore;
    FreeSemaphore++;

    SemaphoreQueue[sem_id].state = SEM_AVAILABLE;
    SemaphoreQueue[sem_id].owner_task = -1;
    SemaphoreQueue[sem_id].priority = priority;
    SemaphoreQueue[sem_id].name = name;

    printf("Semaphore %s created with priority %d\n", name, priority);

    return sem_id;
}

// Wait on a semaphore (P operation)
int SemaphoreWait(int sem_id)
{
    if (sem_id < 0 || sem_id >= MAX_RES)
    {
        printf("ERROR: Invalid semaphore ID\n");
        return -1;
    }

    printf("SemaphoreWait on %s\n", SemaphoreQueue[sem_id].name);

    if (SemaphoreQueue[sem_id].state == SEM_AVAILABLE)
    {
        // Acquire the semaphore
        SemaphoreQueue[sem_id].state = SEM_TAKEN;
        SemaphoreQueue[sem_id].owner_task = RunningTask;

        // Apply priority ceiling protocol
        if (TaskQueue[RunningTask].ceiling_priority < SemaphoreQueue[sem_id].priority)
        {
            TaskQueue[RunningTask].ceiling_priority = SemaphoreQueue[sem_id].priority;
            printf("Priority ceiling raised to %d for task %s\n",
                   SemaphoreQueue[sem_id].priority, TaskQueue[RunningTask].name);
        }

        return 0; // Success
    }
    else
    {
        // Semaphore is taken - block the task
        TaskQueue[RunningTask].state = TASK_WAITING;

        int current_task = RunningTask;
        RunningTask = TaskQueue[current_task].ref;

        printf("Task %s blocked on semaphore %s\n",
               TaskQueue[current_task].name, SemaphoreQueue[sem_id].name);

        if (RunningTask == -1)
        {
            IdleLoop();
        }
        else
        {
            Dispatch(current_task);
        }

        return 0; // Will resume later
    }
}

// Signal a semaphore (V operation)
int SemaphoreSignal(int sem_id)
{
    if (sem_id < 0 || sem_id >= MAX_RES)
    {
        printf("ERROR: Invalid semaphore ID\n");
        return -1;
    }

    printf("SemaphoreSignal on %s\n", SemaphoreQueue[sem_id].name);

    if (SemaphoreQueue[sem_id].state == SEM_TAKEN &&
        SemaphoreQueue[sem_id].owner_task == RunningTask)
    {
        // Release the semaphore
        SemaphoreQueue[sem_id].state = SEM_AVAILABLE;
        SemaphoreQueue[sem_id].owner_task = -1;

        // Reset task priority if needed
        if (TaskQueue[RunningTask].ceiling_priority == SemaphoreQueue[sem_id].priority)
        {
            // Find the highest priority among still-held semaphores
            int max_priority = TaskQueue[RunningTask].priority;

            for (int i = 0; i < MAX_RES; i++)
            {
                if (i != sem_id &&
                    SemaphoreQueue[i].state == SEM_TAKEN &&
                    SemaphoreQueue[i].owner_task == RunningTask &&
                    SemaphoreQueue[i].priority > max_priority)
                {
                    max_priority = SemaphoreQueue[i].priority;
                }
            }

            TaskQueue[RunningTask].ceiling_priority = max_priority;
            printf("Priority restored to %d for task %s\n",
                   max_priority, TaskQueue[RunningTask].name);
        }

        // Check if any waiting tasks should be woken up
        for (int i = 0; i < MAX_TASK; i++)
        {
            if (i != RunningTask &&
                TaskQueue[i].state == TASK_WAITING)
            {
                // In a real implementation, we'd have a wait reason field
                // For now, just assume all WAITING tasks might be waiting on this semaphore

                TaskQueue[i].state = TASK_READY;
                Schedule(i, INSERT_TO_TAIL);

                printf("Task %s woken up after semaphore %s released\n",
                       TaskQueue[i].name, SemaphoreQueue[sem_id].name);
            }
        }

        // Check if we need to be preempted
        if (RunningTask != -1 &&
            TaskQueue[RunningTask].ceiling_priority < TaskQueue[TaskQueue[RunningTask].ref].ceiling_priority)
        {
            int prev_running = RunningTask;
            RunningTask = TaskQueue[RunningTask].ref;
            Dispatch(prev_running);
        }

        return 0; // Success
    }
    else if (SemaphoreQueue[sem_id].state == SEM_TAKEN)
    {
        printf("ERROR: Task %s attempting to release semaphore %s owned by task %s\n",
               TaskQueue[RunningTask].name,
               SemaphoreQueue[sem_id].name,
               TaskQueue[SemaphoreQueue[sem_id].owner_task].name);
        return -1;
    }
    else
    {
        printf("ERROR: Semaphore %s not taken\n", SemaphoreQueue[sem_id].name);
        return -1;
    }
}