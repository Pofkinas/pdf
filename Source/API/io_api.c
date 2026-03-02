/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "io_api.h"

#if defined(ENABLE_IO)
#include <stdio.h>
#include "debug_api.h"
#include "exti_driver.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define MUTEX_TIMEOUT 0U
#define IO_MESSAGE_WAKEUP

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eIoDeviceState {
    eIoDeviceState_First = 0,
    eIoDeviceState_Default = eIoDeviceState_First,
    eIoDeviceState_Init,
    eIoDeviceState_Debounce,
    eIoDeviceState_Last
} eIoDeviceState_t;

typedef enum eIoState {
    eIoState_First = 0,
    eIoState_Default = eIoState_First,
    eIoState_Init,
    eIoState_Started,
    eIoState_Last
} eIoState_t;

typedef struct sIoDynamic {
    eIo_t device;
    eIoDeviceState_t device_state;
    osMutexId_t mutex;
    osEventFlagsId_t callback_flag;
    osTimerId_t debounce_timer;
    bool io_state;
} sIoDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#if defined(DEBUG_IO_API)
CREATE_MODULE_NAME (IO_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_IO_API */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static osThreadId_t g_io_thread_id = NULL;
static osMessageQueueId_t g_io_message_queue_id = NULL;

static bool g_has_polled_io = false;
static eIoState_t g_io_state = eIoState_Default;
static sIoDesc_t g_static_io_desc_lut[eIo_Last] = {0};
static sIoDynamic_t g_dynamic_io_lut[eIo_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void IO_API_Thread (void *arg);
static void IO_API_DebounceTimerCallback (void *context);
static void IO_API_ExtiTriggered (void *context);
static void IO_API_StartDebounceTimer (const eIo_t device);
static bool IO_API_IsGpioStateCorrect (const eIo_t device);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void IO_API_Thread (void *arg) {
    eIo_t device;
    
    while (1) {
        if (osOK == osMessageQueueGet(g_io_message_queue_id, &device, IO_MESSAGE_QUEUE_PRIORITY, g_has_polled_io ? osWaitForever : IO_MESSAGE_QUEUE_TIMEOUT)) {    
            if (g_static_io_desc_lut[device].is_debounce_enable) {
                IO_API_StartDebounceTimer(device);
            }

            continue;
        }

        for (device = eIo_First; device < eIo_Last; device++) {
            if (g_static_io_desc_lut[device].is_debounce_enable && (eIoDeviceState_Debounce == g_dynamic_io_lut[device].device_state)) {
                continue;
            }

            if (g_static_io_desc_lut[device].is_exti) {
                continue;
            }

            if (!IO_API_IsGpioStateCorrect(device)) {
                continue;
            }

            if (g_static_io_desc_lut[device].is_debounce_enable) {
                IO_API_StartDebounceTimer(device);
            } else {
                TRACE_INFO("IO_Thread: IO [%d] triggered\n", device);

                osEventFlagsSet(g_dynamic_io_lut[device].callback_flag, g_static_io_desc_lut[device].triggered_flag);
            }
        }

        osThreadYield();
    }
}

static void IO_API_ExtiTriggered (void *context) {
    if (eIoState_Started != g_io_state) {
        return;
    }
    
    sIoDynamic_t *device = (sIoDynamic_t*) context;
 
    if (!g_static_io_desc_lut[device->device].is_debounce_enable) {
        osEventFlagsSet(device->callback_flag, g_static_io_desc_lut[device->device].triggered_flag);

        return;
    }

    Exti_Driver_DisableIt(g_static_io_desc_lut[device->device].exti_device);
    osMessageQueuePut(g_io_message_queue_id, &device->device, IO_MESSAGE_QUEUE_PRIORITY, 0);

    return;
}

static void IO_API_DebounceTimerCallback (void *context) {
    if (eIoState_Started != g_io_state) {
        return;
    }
    
    sIoDynamic_t *debounce_io = (sIoDynamic_t*) context;
    bool debounce_status = true;

    if (eIoDeviceState_Debounce != debounce_io->device_state) {
        TRACE_WRN("Debounce: exit early, state [%d]\n", debounce_io->device_state);
        
        return;
    }

    if (!IO_API_IsGpioStateCorrect(debounce_io->device)) {
        TRACE_WRN("Debounce: GPIO state is incorrect [%d]\n", debounce_io->device);
        
        debounce_status = false;
    }

    if (osOK != osMutexAcquire(debounce_io->mutex, MUTEX_TIMEOUT)) {
        debounce_status = false;
    }

    if (g_static_io_desc_lut[debounce_io->device].is_exti) {
        if (!Exti_Driver_ClearFlag(g_static_io_desc_lut[debounce_io->device].exti_device)) {
            debounce_status = false;
        }

        Exti_Driver_EnableIt(g_static_io_desc_lut[debounce_io->device].exti_device);
    }

    debounce_io->device_state = eIoDeviceState_Init;

    osMutexRelease(debounce_io->mutex);

    if (!debounce_status) {  
        TRACE_WRN("Debounce: IO [%d] debounce failed\n", debounce_io->device);

        return;
    }

    osEventFlagsSet(debounce_io->callback_flag, g_static_io_desc_lut[debounce_io->device].triggered_flag);

    TRACE_INFO("Debounce: IO [%d] triggered\n", debounce_io->device);

    return;
}

static void IO_API_StartDebounceTimer (const eIo_t device) {
    if (!IO_Config_IsCorrectIo(device)) {
        return;
    }

    if (eIoDeviceState_Init != g_dynamic_io_lut[device].device_state) {
        return;
    }

    if (osOK != osMutexAcquire(g_dynamic_io_lut[device].mutex, MUTEX_TIMEOUT)) {
        return;
    }

    g_dynamic_io_lut[device].device_state = eIoDeviceState_Debounce;

    osMutexRelease(g_dynamic_io_lut[device].mutex);

    osTimerStart(g_dynamic_io_lut[device].debounce_timer, g_static_io_desc_lut[device].debounce_period);

    return;
}

static bool IO_API_IsGpioStateCorrect (const eIo_t device) {
    if (!IO_Config_IsCorrectIo(device)) {
        return false;
    }

    if (eIoDeviceState_Default == g_dynamic_io_lut[device].device_state) {
        return false;
    }

    if (!GPIO_Driver_ReadPin(g_static_io_desc_lut[device].gpio_pin, &g_dynamic_io_lut[device].io_state)) {
        return false;
    }

    switch (g_static_io_desc_lut[device].active_state) {
        case eActiveState_Low: {
            if (g_dynamic_io_lut[device].io_state) {
                return false;
            }
        } break;
        case eActiveState_High: {
            if (!g_dynamic_io_lut[device].io_state) {
                return false;
            }
        } break;
        case eActiveState_Both: {
        } break;
        default: {
            return false;
        }
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool IO_API_Init (const eIo_t device, osEventFlagsId_t event_flags_id) {
    if (!IO_Config_IsCorrectIo(device)) {
        return false;
    }

    if (eIoDeviceState_Default != g_dynamic_io_lut[device].device_state) {
        return true;
    }

    if (NULL == event_flags_id) {
        return false;
    }

    const sIoDesc_t *desc = IO_Config_GetIoDesc(device);

    if (NULL == desc) {
        return false;
    }

    g_static_io_desc_lut[device] = *desc;

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    if (!Exti_Driver_InitDevice(g_static_io_desc_lut[device].exti_device, &IO_API_ExtiTriggered, &g_dynamic_io_lut[device])) {
        return false;
    }

    if (g_static_io_desc_lut[device].is_debounce_enable) {
        g_dynamic_io_lut[device].debounce_timer = osTimerNew(IO_API_DebounceTimerCallback, osTimerOnce, &g_dynamic_io_lut[device], &g_static_io_desc_lut[device].debounce_timer_attributes);
    
        if (NULL == g_dynamic_io_lut[device].debounce_timer) {
            return false;
        }
    }

    if (NULL == g_dynamic_io_lut[device].mutex) {
        g_dynamic_io_lut[device].mutex = osMutexNew(&g_static_io_desc_lut[device].mutex_attributes);
    
        if (NULL == g_dynamic_io_lut[device].mutex) {
            return false;
        }
    }

    g_dynamic_io_lut[device].callback_flag = event_flags_id;

    if (!g_static_io_desc_lut[device].is_exti) {
        g_has_polled_io = true;
    }

    g_dynamic_io_lut[device].device_state = eIoDeviceState_Init;
    g_dynamic_io_lut[device].device = device;

    if (eIoState_Default == g_io_state) {
        g_io_state = eIoState_Init;
    }

    return true;
}

bool IO_API_Start (void) {
    if (eIoState_Default == g_io_state) {
        return false;
    }

    if (eIoState_Started == g_io_state) {
        return true;
    }

    if (NULL == g_io_thread_id) {
        g_io_thread_id = osThreadNew(IO_API_Thread, NULL, &g_io_thread_attributes);

        if (NULL == g_io_thread_id) {
            return false;
        }
    }
    
    if (NULL == g_io_message_queue_id) {
        g_io_message_queue_id = osMessageQueueNew(IO_MESSAGE_CAPACITY, sizeof(eIo_t), &g_io_message_queue_attributes);
    
        if (NULL == g_io_message_queue_id) {
            return false;
        }
    }

    for (eIo_t device = eIo_First; device < eIo_Last; device++) {
        if (g_static_io_desc_lut[device].is_exti) {
            if (!Exti_Driver_EnableIt(g_static_io_desc_lut[device].exti_device)) {
                return false;
            }
        }

        g_dynamic_io_lut[device].device_state = eIoDeviceState_Init;
    }

    g_io_state = eIoState_Started;

    return true;
}

bool IO_API_Stop (void) {
    if (eIoState_Default == g_io_state) {
        return false;
    } else if (eIoState_Init == g_io_state) {
        return true;
    }

    osThreadTerminate(g_io_thread_id);
    g_io_thread_id = NULL;

    osMessageQueueDelete(g_io_message_queue_id);
    g_io_message_queue_id = NULL;

    for (eIo_t device = eIo_First; device < eIo_Last; device++) {
        if (g_static_io_desc_lut[device].is_exti) {
            Exti_Driver_DisableIt(g_static_io_desc_lut[device].exti_device);
        }

        g_dynamic_io_lut[device].device_state = eIoDeviceState_Init;
    }

    g_io_state = eIoState_Init;

    return true;
}

bool IO_API_ReadPinState (const eIo_t device, bool *pin_state) {
    if (!IO_Config_IsCorrectIo(device)) {
        return false;
    }

    if (NULL == pin_state) {
        return false;
    }

    if (eIoDeviceState_Default == g_dynamic_io_lut[device].device_state) {
        return false;
    }

    return (GPIO_Driver_ReadPin(g_static_io_desc_lut[device].gpio_pin, pin_state));
}

#endif /* ENABLE_IO */
