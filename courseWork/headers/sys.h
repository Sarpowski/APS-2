/****************************************/
/*               sys.h                   /          
/****************************************/

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

extern TTask TaskQueue[MAX_TASK];
extern TResource ResourceQueue[MAX_RES];
extern TEvent EventQueue[MAX_EVENT];

extern int RunningTask;
extern int FreeTask;
extern int FreeResource;
extern int FreeEvent;

void Schedule(int task,int mode);

void Dispatch(int task);

void CheckDeadlines(void);