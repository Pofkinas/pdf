/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "led_app.h"

#if defined(ENABLE_LED) || defined(ENABLE_PWM_LED)

#include <stddef.h>
#include "cmsis_os2.h"
#include "cli_app.h"
#include "debug_api.h"
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

#ifdef DEBUG_LED_APP
CREATE_MODULE_NAME (LED_APP)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_LED_APP */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sLedCommandDesc_t g_received_task = {.task = eLedTask_Last, .data = NULL};
static bool g_is_initialized = false;

static osThreadId_t g_led_thread_id = NULL;
static osMessageQueueId_t g_led_message_queue_id = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void LED_APP_Thread (void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void LED_APP_Thread (void *arg) {
    while (1) {
        if (osMessageQueueGet(g_led_message_queue_id, &g_received_task, LED_APP_MESSAGE_QUEUE_PRIORITY, LED_APP_MESSAGE_QUEUE_TIMEOUT) != osOK) {
            continue;
        }

        if (g_received_task.data == NULL) {
            TRACE_ERR("No arguments\n");
        }
        switch (g_received_task.task) {
            #ifdef ENABLE_LED
            case eLedTask_Set: {
                sLedCommon_t *arguments = (sLedCommon_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_Config_IsCorrectLed(arguments->led)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }
                
                if (!LED_API_TurnOn(arguments->led)) {
                    TRACE_ERR("LED Turn On Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Set\n", arguments->led);

                Heap_API_Free(arguments);    
            } break;
            case eLedTask_Reset: {
                sLedCommon_t *arguments = (sLedCommon_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_Config_IsCorrectLed(arguments->led)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_TurnOff(arguments->led)) {
                    TRACE_ERR("LED Turn Off Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Reset\n", arguments->led);

                Heap_API_Free(arguments);
            } break;
            case eLedTask_Toggle: {
                sLedCommon_t *arguments = (sLedCommon_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_Config_IsCorrectLed(arguments->led)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_Toggle(arguments->led)) {
                    TRACE_ERR("LED Toggle Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Toggle\n", arguments->led);

                Heap_API_Free(arguments);
            } break;
            case eLedTask_Blink: {
                sLedBlink_t *arguments = (sLedBlink_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_Config_IsCorrectLed(arguments->led)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectBlinkTime(arguments->blink_time)) {
                    TRACE_ERR("Invalid blink time\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectBlinkFrequency(arguments->blink_frequency)) {
                    TRACE_ERR("Invalid blink frequency\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_Blink(arguments->led, arguments->blink_time, arguments->blink_frequency)) {
                    TRACE_ERR("LED Blink Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Blink %d s, @ %d Hz\n", arguments->led, arguments->blink_time, arguments->blink_frequency);

                Heap_API_Free(arguments);
            } break;
            #endif /* ENABLE_LED */

            #ifdef ENABLE_PWM_LED
            case eLedTask_Set_Brightness: {
                sLedSetBrightness_t *arguments = (sLedSetBrightness_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_Config_IsCorrectPwmLed(arguments->led)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectDutyCycle(arguments->led, arguments->duty_cycle)) {
                    TRACE_ERR("Invalid duty cycle\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_Set_Brightness(arguments->led, arguments->duty_cycle)) {
                    TRACE_ERR("LED Set Brightness Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Pwm Led Brightness %d\n", arguments->led, arguments->duty_cycle);

                Heap_API_Free(arguments);
            } break;
            case eLedTask_Pulse: {
                sLedPulse_t *arguments = (sLedPulse_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_Config_IsCorrectPwmLed(arguments->led)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectPulseTime(arguments->pulse_time)) {
                    TRACE_ERR("Invalid pulse time\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectPulseFrequency(arguments->pulse_frequency)) {
                    TRACE_ERR("Invalid pulse frequency\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_Pulse(arguments->led, arguments->pulse_time, arguments->pulse_frequency)) {
                    TRACE_ERR("LED Pulse Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Pwm Led %d Pulse %d s, @ %d Hz\n", arguments->led, arguments->pulse_time, arguments->pulse_frequency);

                Heap_API_Free(arguments);
            } break;
            #endif /* ENABLE_PWM_LED */
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

bool LED_APP_Init (void) {
    if (g_is_initialized) {
        return true;
    }

    if (!LED_API_Init()) {
        return false;
    }

    g_led_message_queue_id = osMessageQueueNew(CLI_COMMAND_MESSAGE_CAPACITY, sizeof(sLedCommandDesc_t), &g_led_message_queue_attributes);
    
    if (g_led_message_queue_id == NULL) {
        return false;
    }

    g_led_thread_id = osThreadNew(LED_APP_Thread, NULL, &g_led_thread_attributes);

    if (g_led_thread_id == NULL) {
        return false;
    }

    g_is_initialized = true;

    return g_is_initialized;
}

bool LED_APP_Add_Task (sLedCommandDesc_t *task_to_message_queue) {
    if (task_to_message_queue == NULL) {
        return false;
    }

    if (g_led_message_queue_id == NULL){
        return false;
    }

    if (osMessageQueuePut(g_led_message_queue_id, task_to_message_queue, LED_APP_MESSAGE_QUEUE_PRIORITY, LED_APP_MESSAGE_QUEUE_TIMEOUT) != osOK) {
        return false;
    }

    return true;
}

#endif /* defined(ENABLE_LED) || defined(ENABLE_PWM_LED) */
