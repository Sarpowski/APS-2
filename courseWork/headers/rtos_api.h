/****************************************/
/*           rtos_api.h                 */
/****************************************/

// Task declaration macros
#define DeclareTask(TaskID, priority) \
    TASK(TaskID); \
    enum {TaskID##prior = priority}

// Resource declaration macro
#define DeclareResource(ResID, priority) \
    enum {ResID = priority}

// Event declaration macro
#define DeclareEvent(EventID) \
    enum {EventID}

// Task definition macro
#define TASK(TaskID) void TaskID(void)

// Task function type
typedef void TTaskCall(void);

// Task management functions (POSIX-like)
void ActivateTask(TTaskCall entry, int priority, char* name);
void TerminateTask(void);
void DelayTask(int ticks);  // Delay task execution

// RTOS control functions
int StartOS(TTaskCall entry, int priority, char* name);
void ShutdownOS(void);
void IdleLoop(void);  // System idle loop

// Resource management (simple semaphores)
void GetResource(int priority, char* name);   // Acquire semaphore
void ReleaseResource(int priority, char* name);  // Release semaphore

// Event management
void SetEvent(int event_id, char* name);      // Set event
void ClearEvent(int event_id, char* name);    // Clear event
void WaitEvent(int event_id, char* name);     // Wait for event

// POSIX-like functions
int CreateTask(TTaskCall entry, int priority, char* name);  // Create but don't activate
int SuspendTask(int task_id);                 // Suspend a task
int ResumeTask(int task_id);                  // Resume a suspended task

// RMA specific functions
void SetTaskPeriod(int task_id, int period);  // Set the period for a task
void SetTaskDeadline(int task_id, int deadline);  // Set the deadline for a task