/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_app.h"

#ifdef ENABLE_MOTOR

#include <stddef.h>
#include "cmsis_os2.h"
#include "cli_app.h"
#include "debug_api.h"
#include "motor_api.h"
#include "heap_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_MOTOR_APP
CREATE_MODULE_NAME (MOTOR_APP)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_MOTOR_APP */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sMotorCommandDesc_t g_received_task = {.task = eMotorTask_Last, .data = NULL};
static bool g_is_initialized = false; 

static osThreadId_t g_motor_thread_id = NULL;
static osMessageQueueId_t g_motor_message_queue_id = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void Motor_APP_Thread (void *arg); 

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void Motor_APP_Thread (void *arg) {
    while (1) {
        if (osMessageQueueGet(g_motor_message_queue_id, &g_received_task, MOTOR_MESSAGE_QUEUE_PRIORITY, MOTOR_MESSAGE_QUEUE_TIMEOUT) != osOK) {
            continue;
        }

        switch (g_received_task.task) {
            case eMotorTask_Set: {
                if (g_received_task.data == NULL) {
                    TRACE_ERR("No arguments\n");

                    break;
                }

                sMotorSet_t *arguments = (sMotorSet_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }
            
                if (!Motor_API_IsCorrectSpeed(arguments->speed)) {
                    TRACE_ERR("Invalid Motor Speed\n");

                    Heap_API_Free(arguments);

                    break;
                }
            
                if (!Motor_Config_IsCorrectDirection(arguments->direction)) {
                    TRACE_ERR("Invalid Motor direction\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!Motor_API_SetSpeed(arguments->speed, arguments->direction)) {
                    TRACE_ERR("Motor Set Speed Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Motors @ Speed %d, Dir %d\n", arguments->speed, arguments->direction);

                Heap_API_Free(arguments);    
            } break;
            case eMotorTask_Stop: {
                if (!Motor_API_StopAllMotors()) {
                    TRACE_ERR("Motor Stop Failed\n");

                    break;
                }

                TRACE_INFO("Motors Stopped\n");
            } break;
            default: {
                TRACE_ERR("Task not found\n");
            } break;
        }
    }

    osThreadYield();
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Motor_APP_Init (void) {
    if (g_is_initialized) {
        return true;
    }

    if (!Motor_API_Init()) {
        return false;
    }

    g_motor_message_queue_id = osMessageQueueNew(MOTOR_MESSAGE_QUEUE_CAPACITY, sizeof(sMotorCommandDesc_t), &g_motor_message_queue_attributes);

    if (g_motor_message_queue_id == NULL) {
        return false;
    }

    g_motor_thread_id = osThreadNew(Motor_APP_Thread, NULL, &g_motor_thread_attributes);

    if (g_motor_thread_id == NULL) {
        return false;
    }

    g_is_initialized = true;

    return g_is_initialized;
}

bool Motor_APP_Add_Task (sMotorCommandDesc_t *task_to_message_queue) {
    if (task_to_message_queue == NULL) {
        return false;
    }

    if (g_motor_message_queue_id == NULL){
        return false;
    }

    if (osMessageQueuePut(g_motor_message_queue_id, task_to_message_queue, MOTOR_MESSAGE_QUEUE_PRIORITY, MOTOR_MESSAGE_QUEUE_TIMEOUT) != osOK) {
        return false;
    }

    return true;
}

#endif /* ENABLE_MOTOR */
