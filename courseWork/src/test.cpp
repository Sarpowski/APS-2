/*******************************/
/*           test.cpp           */
/*******************************/

#include <stdio.h>
#include <stdlib.h>

#include "rtos_api.h"

// Declare tasks with RMA priorities (lower number = higher rate = higher priority)
DeclareTask(TaskIdle, 16);     // Lowest priority
DeclareTask(TaskHigh, 1);      // Highest priority
DeclareTask(TaskMedium, 5);
DeclareTask(TaskLow, 10);

// Declare resources (semaphores)
DeclareResource(Res1, 12);
DeclareResource(Res2, 8);

// Declare events
DeclareEvent(Event1);
DeclareEvent(Event2);

// Test functions
void TestTaskPreemption();
void TestResourceManagement();
void TestEventManagement();
void TestRMA();

// Test time variables (for RMA demonstration)
extern int SystemTick;
extern int TaskPeriods[MAX_TASK];
extern int TaskDeadlines[MAX_TASK];

// Main test function
int test(void)
{
    printf("Starting RTOS Test Suite\n");

    // Test 1: Basic task management
    printf("\n=== Test 1: Basic Task Management ===\n");
    char name[] = "TaskIdle";
    StartOS(TaskIdle, TaskIdleprior, name);

    // Other tests will be triggered by TaskIdle

    ShutdownOS();

    return 0;
}

// Idle task - runs various tests
TASK(TaskIdle)
{
    printf("TaskIdle: Starting tests\n");

    // Test task preemption
    TestTaskPreemption();

    // Test resource management
    TestResourceManagement();

    // Test event management
    TestEventManagement();

    // Test RMA scheduling
    TestRMA();

    printf("TaskIdle: All tests completed\n");
    TerminateTask();
}

// High priority task
TASK(TaskHigh)
{
    printf("TaskHigh: Running\n");
    printf("TaskHigh: This demonstrates preemption - running before lower priority tasks complete\n");

    // Release a resource to test priority inheritance
    if (TaskQueue[RunningTask].ceiling_priority > TaskHighprior)
    {
        char resName[] = "Res1";
        ReleaseResource(Res1, resName);
    }

    TerminateTask();
}

// Medium priority task
TASK(TaskMedium)
{
    printf("TaskMedium: Running\n");

    // Test resource acquisition (priority ceiling)
    char resName[] = "Res1";
    GetResource(Res1, resName);

    printf("TaskMedium: Acquired resource\n");

    // Activate high priority task to test preemption during resource holding
    char taskName[] = "TaskHigh";
    ActivateTask(TaskHigh, TaskHighprior, taskName);

    printf("TaskMedium: Continuing after TaskHigh\n");

    // Release resource
    ReleaseResource(Res1, resName);

    printf("TaskMedium: Resource released\n");

    TerminateTask();
}

// Low priority task
TASK(TaskLow)
{
    printf("TaskLow: Running\n");

    // Test event waiting
    printf("TaskLow: Waiting for Event1\n");
    char eventName[] = "Event1";
    WaitEvent(Event1, eventName);

    printf("TaskLow: Event1 received\n");

    TerminateTask();
}

// Test task preemption
void TestTaskPreemption()
{
    printf("\n--- Testing Task Preemption ---\n");

    // Create all tasks but don't activate yet
    int highTask = CreateTask(TaskHigh, TaskHighprior, (char*)"TaskHigh");
    int medTask = CreateTask(TaskMedium, TaskMediumprior, (char*)"TaskMedium");
    int lowTask = CreateTask(TaskLow, TaskLowprior, (char*)"TaskLow");

    // Activate low priority task
    ResumeTask(lowTask);

    // Activate medium priority task (should preempt low)
    ResumeTask(medTask);

    // TaskHigh will be activated by TaskMedium

    printf("--- Task Preemption Test Complete ---\n");
}

// Test resource management
void TestResourceManagement()
{
    printf("\n--- Testing Resource Management ---\n");

    // Create tasks
    int highTask = CreateTask(TaskHigh, TaskHighprior, (char*)"TaskHigh");
    int medTask = CreateTask(TaskMedium, TaskMediumprior, (char*)"TaskMedium");

    // Activate medium task which will acquire a resource
    ResumeTask(medTask);

    printf("--- Resource Management Test Complete ---\n");
}

// Test event management
void TestEventManagement()
{
    printf("\n--- Testing Event Management ---\n");

    // Create tasks
    int lowTask = CreateTask(TaskLow, TaskLowprior, (char*)"TaskLow");

    // Activate low task which will wait for an event
    ResumeTask(lowTask);

    // Set the event that TaskLow is waiting for
    printf("Main: Setting Event1\n");
    SetEvent(Event1, (char*)"Event1");

    printf("--- Event Management Test Complete ---\n");
}

// Test Rate Monotonic Algorithm scheduling
void TestRMA()
{
    printf("\n--- Testing RMA Scheduling ---\n");

    // Create periodic tasks with different periods
    int highTask = CreateTask(TaskHigh, TaskHighprior, (char*)"TaskHigh");
    int medTask = CreateTask(TaskMedium, TaskMediumprior, (char*)"TaskMedium");
    int lowTask = CreateTask(TaskLow, TaskLowprior, (char*)"TaskLow");

    // Set periods according to RMA (higher priority = shorter period)
    SetTaskPeriod(highTask, 2);  // Run every 2 ticks
    SetTaskPeriod(medTask, 5);   // Run every 5 ticks
    SetTaskPeriod(lowTask, 10);  // Run every 10 ticks

    // Set deadlines equal to periods
    SetTaskDeadline(highTask, 2);
    SetTaskDeadline(medTask, 5);
    SetTaskDeadline(lowTask, 10);

    // Activate all tasks
    ResumeTask(highTask);
    ResumeTask(medTask);
    ResumeTask(lowTask);

    // Let them run for a while
    printf("Letting periodic tasks run for 20 ticks...\n");
    for (int i = 0; i < 20; i++)
    {
        SystemTick++;
        CheckDeadlines();
    }

    printf("--- RMA Scheduling Test Complete ---\n");
}