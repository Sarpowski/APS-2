/****************************************/
/*           defs.h               */
/****************************************/

#define MAX_TASK 32
#define MAX_RES   16
#define MAX_EVENT 16

enum T_TaskState{
    TASK_RUNNING,
    TASK_READY,
    TASK_SUSPENDED,
    TASK_WAITING
};


// Scheduling constants
#define INSERT_TO_TAIL 1
#define INSERT_TO_HEAD 0

// Event status flags
#define EVENT_CLEAR 0
#define EVENT_SET 1