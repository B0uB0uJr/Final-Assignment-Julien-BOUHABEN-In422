/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/******************************************************************************
 * NOTE 1: The FreeRTOS demo threads will not be running continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Linux port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Linux
 * port for further information:
 * https://freertos.org/FreeRTOS-simulator-for-Linux.html
 *
 * NOTE 2:  This project provides two demo applications.  A simple blinky style
 * project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the simply blinky version.  Console output
 * is used in place of the normal LED toggling.
 *
 * NOTE 3:  This file only contains the source code that is specific to the
 * basic demo.  Generic functions, such FreeRTOS hook functions, are defined
 * in main.c.
 ******************************************************************************
 *
 * main_blinky() creates one queue, one software timer, and two tasks.  It then
 * starts the scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  It uses vTaskDelayUntil() to create a periodic task that sends
 * the value 100 to the queue every 200 milliseconds (please read the notes
 * above regarding the accuracy of timing under Linux).
 *
 * The Queue Send Software Timer:
 * The timer is an auto-reload timer with a period of two seconds.  The timer's
 * callback function writes the value 200 to the queue.  The callback function
 * is implemented by prvQueueSendTimerCallback() within this file.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() waits for data to arrive on the queue.
 * When data is received, the task checks the value of the data, then outputs a
 * message to indicate if the data came from the queue send task or the queue
 * send software timer.
 *
 * Expected Behaviour:
 * - The queue send task writes to the queue every 200ms, so every 200ms the
 *   queue receive task will output a message indicating that data was received
 *   on the queue from the queue send task.
 * - The queue send software timer has a period of two seconds, and is reset
 *   each time a key is pressed.  So if two seconds expire without a key being
 *   pressed then the queue receive task will output a message indicating that
 *   data was received on the queue from the queue send software timer.
 *
 * NOTE:  Console input and output relies on Linux system calls, which can
 * interfere with the execution of the FreeRTOS Linux port. This demo only
 * uses Linux system call occasionally. Heavier use of Linux system calls
 * may crash the port.
 */

#include <stdio.h>
#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"
#include <math.h>


/* Priorities at which the tasks are created. */
#define mainQUEUE_LENGTH           (2)
#define TASK1_PERIOD_MS            (166 / portTICK_PERIOD_MS)
#define TASK2_PERIOD_MS            (170 / portTICK_PERIOD_MS)
#define TASK3_PERIOD_MS            (186/ portTICK_PERIOD_MS)
#define TASK4_PERIOD_MS            (166 / portTICK_PERIOD_MS)

#define TASK1_PRIORITY             (tskIDLE_PRIORITY + 1)
#define TASK2_PRIORITY             (tskIDLE_PRIORITY + 2)
#define TASK3_PRIORITY             (tskIDLE_PRIORITY + 3)
#define TASK4_PRIORITY             (tskIDLE_PRIORITY + 4)


/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void vPeriodicTask1(void *params);
static void vPeriodicTask2(void *params);
static void vPeriodicTask3(void *params);
static void vPeriodicTask4(void *params);

/*-----------------------------------------------------------*/

void ipsa_sched(void)
{
    /* Create the queue. */
    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));

    if (xQueue != NULL)
    {
        /* Start the tasks as described in the comments at the top of this file. */
        xTaskCreate(vPeriodicTask1, "TX1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, NULL);
        xTaskCreate(vPeriodicTask2, "TX2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, NULL);
        xTaskCreate(vPeriodicTask3, "TX3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, NULL);
        xTaskCreate(vPeriodicTask4, "TX4", configMINIMAL_STACK_SIZE, NULL, TASK4_PRIORITY, NULL);

        /* Start the scheduler. */
        vTaskStartScheduler();
    }

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */
    for (;;)
    {
    }
}

/*-----------------------------------------------------------*/

void vPeriodicTask1(void *params)
{
    for (;;)
    {
        printf("Working\n");
        vTaskDelay(TASK1_PERIOD_MS);
    }
}

void vPeriodicTask2(void *params)
{
    float fahrenheit = 90.0f; 

    for (;;)
    {
        float celsius = (fahrenheit - 32.0f) * 5.0f / 9.0f;

        printf("%f °F = %f °C\n", fahrenheit, celsius);
        vTaskDelay(TASK2_PERIOD_MS);
    }
}

void vPeriodicTask3(void *params)
{
    long int num1 = 1432573583;
    long int num2 = 9837362847;
    long int result = 0;

    for (;;)
    {
        result = num1 * num2;

        printf("%ld x %ld = %ld\n", num1,num2,result);
        vTaskDelay(TASK3_PERIOD_MS);
    }
}

void vPeriodicTask4(void *params)
{
    int list[50];
    int element_to_find = 25;
    int low, high, mid;
    int target = 0;

    for(int i = 0; i < 50; i++) {
        list[i] = i + 1;
    }

    low = 0;
    high = 49;

    for (;;)
    {
        while (low <= high)
        {
            mid = (low + high) / 2;

            if (list[mid] == element_to_find)
            {
                target = 1;
                printf("%d found at position %d\n", element_to_find,mid);
                break;
            }
            else if (list[mid] < element_to_find)
            {
                low = mid + 1;
            }
            else
            {
                high = mid - 1;
            }
        }

        if target == 0
        {
            printf("Element not found\n");
        }

        vTaskDelay(TASK4_PERIOD_MS);
    }
}