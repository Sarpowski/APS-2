#include "defs.h"

typedef struct Type_Task
{
    int ref;
    int priority;
    int ceiling_priority;
    int state;
    int waiting_event;
    void (*entry)(void);
    char* name;

} TTask;

typedef struct Type_resource
{
    int task;
    int priority;
    char* name;

} TResource;

typedef struct Type_event{
    int status;
    char* name;
} TEvent;

// Semaphore states
#define SEM_AVAILABLE 0
#define SEM_TAKEN 1

typedef struct Type_semaphore
{
    int state;          // SEM_AVAILABLE or SEM_TAKEN
    int owner_task;     // Task ID of owner (-1 if available)
    int priority;       // Priority level for ceiling protocol
    char* name;         // Name of the semaphore
} TSemaphore;

extern TTask TaskQueue[MAX_TASK];
extern TResource ResourceQueue[MAX_RES];
extern TEvent EventQueue[MAX_EVENT];
extern TSemaphore SemaphoreQueue[MAX_RES];

extern int RunningTask;
extern int FreeTask;
extern int FreeResource;
extern int FreeEvent;
extern int FreeSemaphore;

void Schedule(int task,int mode);

void Dispatch(int task);

void CheckDeadlines(void);