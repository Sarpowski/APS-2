/*******************************/
/*           main.cpp           */
/*******************************/

#include <stdio.h>
#include <stdlib.h>
#include "rtos_api.h"

// Declare the test function from test.cpp
extern int test(void);
/*

int main(void)
{
    // Call the test function that contains the RTOS initialization
    return test();
}*/

#include <stdio.h>
#include <stdlib.h>
#include "rtos_api.h"

// Declare the test function from test.cpp
extern int test(void);

int main(void)
{
    printf("DEBUG: Starting main function\n");

    // Call the test function that contains the RTOS initialization
    int result = test();

    printf("DEBUG: Test function returned %d\n", result);

    return result;
}