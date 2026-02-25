/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_app.h"

#if defined(ENABLE_MOTOR)

#include <stddef.h>
#include "cmsis_os2.h"
#include "cli_app.h"
#include "debug_api.h"
#include "motor_api.h"
#include "heap_api.h"
#include "float_parts.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#if defined(DEBUG_MOTOR_APP)
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
        if (osOK != osMessageQueueGet(g_motor_message_queue_id, &g_received_task, MOTOR_MESSAGE_QUEUE_PRIORITY, MOTOR_MESSAGE_QUEUE_TIMEOUT)) {
            continue;
        }

        switch (g_received_task.task) {
            case eMotorTask_Set: {
                if (NULL == g_received_task.data) {
                    TRACE_ERR("No arguments\n");

                    break;
                }

                sMotorSet_t *arguments = (sMotorSet_t*) g_received_task.data;

                if (NULL == arguments) {
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

                if (!Motor_API_IsCorrectMode(arguments->mode)) {
                    TRACE_ERR("Invalid Motor control mode\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!Motor_API_SetMotors(arguments->speed, arguments->direction, arguments->mode)) {
                    TRACE_ERR("Motor Set Speed Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Motors @ Speed [%d.%03u], Dir [%d], Mode [%d]\n", FLOAT_INTEGER_PART(arguments->speed), FLOAT_FRACTIONAL_PART(arguments->speed, 3), arguments->direction, arguments->mode);

                Heap_API_Free(arguments);    
            } break;
            case eMotorTask_Stop: {
                if (!Motor_API_StopAllMotors()) {
                    TRACE_ERR("Motor Stop Failed\n");

                    break;
                }

                TRACE_INFO("Motors Stopped\n");
            } break;
            #if defined(ENABLE_PID_CONTROL)
            case eMotorTask_SetRpm: {
                if (NULL == g_received_task.data) {
                    TRACE_ERR("No arguments\n");

                    break;
                }

                sMotorSetRpm_t *arguments = (sMotorSetRpm_t*) g_received_task.data;

                if (NULL == arguments) {
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!Motor_API_IsCorrectRpm(arguments->target_rpm)) {
                    TRACE_ERR("Invalid Motor target RPM\n");

                    Heap_API_Free(arguments);

                    break;
                }
            
                if (!Motor_Config_IsCorrectMotor(arguments->motor)) {
                    TRACE_ERR("Invalid Motor\n");

                    Heap_API_Free(arguments);

                    break;
                }
            
                if (!Motor_API_IsCorrectMode(arguments->mode)) {
                    TRACE_ERR("Invalid Motor control mode\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!Motor_API_SetTargetRpm(arguments->motor, arguments->target_rpm, arguments->mode)) {
                    TRACE_ERR("Motor Set Target RPM Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Motor [%d] @ RPM [%f], Mode [%d]\n", arguments->motor, arguments->target_rpm, arguments->mode);

                Heap_API_Free(arguments);    
            } break;
            #endif /* ENABLE_PID_CONTROL */
            default: {
                TRACE_ERR("Task not found\n");
            } break;
        }
    }
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

    if (NULL == g_motor_message_queue_id) {
        return false;
    }

    g_motor_thread_id = osThreadNew(Motor_APP_Thread, NULL, &g_motor_thread_attributes);

    if (NULL == g_motor_thread_id) {
        return false;
    }

    g_is_initialized = true;

    return g_is_initialized;
}

bool Motor_APP_AddTask (sMotorCommandDesc_t *task_to_message_queue) {
    if (NULL == task_to_message_queue) {
        return false;
    }

    if (NULL == g_motor_message_queue_id) {
        return false;
    }

    if (osOK != osMessageQueuePut(g_motor_message_queue_id, task_to_message_queue, MOTOR_MESSAGE_QUEUE_PRIORITY, MOTOR_MESSAGE_QUEUE_TIMEOUT)) {
        return false;
    }

    return true;
}

#endif /* ENABLE_MOTOR */
