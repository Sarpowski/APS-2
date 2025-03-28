/****************************************/
/*        global.cpp                    */
/****************************************/

#include "sys.h"

// System queues
TTask TaskQueue[MAX_TASK];          // Task queue
TResource ResourceQueue[MAX_RES];    // Resource queue
TEvent EventQueue[MAX_EVENT];        // Event queue

// System state variables
int RunningTask = -1;                // No running task initially
int FreeTask = 0;                    // First free task slot
int FreeResource = 0;                // First free resource slot
int FreeEvent = 0;                   // First free event slot

// RMA specific variables
int SystemTick = 0;                  // System tick counter
int TaskPeriods[MAX_TASK];           // Array to store task periods
int TaskDeadlines[MAX_TASK];         // Array to store task deadlines
int TaskLastRun[MAX_TASK];           // Last run time for each task