/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "ws2812b_api.h"

#ifdef ENABLE_WS2812B
#include "cmsis_os2.h"
#include "heap_api.h"
#include "debug_api.h"
#include "ws2812b_driver.h"
#include "timer_driver.h"
#include "pwm_driver.h"
#include "gpio_driver.h"

#include "animation_solidcolor.h"
#include "animation_segmentfill.h"
#include "animation_rainbow.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define TRANSFER_SUCCESS_FLAG 0x01U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eWs2812bState {
    eWs2812bState_First = 0,
    eWs2812bState_Idle = eWs2812bState_First,
    eWs2812bState_Building,
    eWs2812bState_Running,
    eWs2812bState_Updating,
    eWs2812bState_Last
} eWs2812bState_t;

typedef struct sWs2812bSequence {
    eLedAnimation_t animation;
    void *data;
    struct sWs2812bSequence *next;
} sWs2812bSequence_t;

typedef struct sWs2812bDynamicDesc {
    eWs2812b_t device;
    uint8_t *led_data;
    size_t led_count;
    eWs2812bState_t led_state;
    sWs2812bSequence_t *dynamic_animations;
    sWs2812bSequence_t *current_animation;
    osTimerId_t timer;
    osMutexId_t mutex;
    osEventFlagsId_t flag;
} sWs2812bApiDynamicDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_WS2812B_API
CREATE_MODULE_NAME (WS2812B_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_WS2812B_API */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
static bool g_ws2812b_api_is_init = false;

static sWs2812bControlDesc_t g_ws2812b_api_static_lut[eWs2812b_Last] = {0};
static sWs2812bApiDynamicDesc_t g_ws2812b_api_dynamic_lut[eWs2812b_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void WS2812B_API_TimerCallback (void *arg);
static bool WS2812B_API_Update (const eWs2812b_t device);
static void WS2812B_API_DriverCallback (void *context, const eLedTransferState_t transfer_state);
static bool WS2812B_API_BuildStaticAnimation (const sLedAnimationDesc_t *static_animation_data);
static bool WS2812B_API_QueueDynamicAnimation (const sLedAnimationDesc_t *dynamic_animation_data);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void WS2812B_API_TimerCallback (void *arg) {
    if (arg == NULL) {
        return;
    }

    sWs2812bApiDynamicDesc_t *timer_arg = (sWs2812bApiDynamicDesc_t *) arg;

    if (timer_arg->led_state != eWs2812bState_Running) {
        if (osMutexAcquire(timer_arg->mutex, MUTEX_TIMEOUT) != osOK) {
            return;
        }
            
        timer_arg->led_state = eWs2812bState_Running;
    }

    timer_arg->current_animation = timer_arg->dynamic_animations;

    while (timer_arg->current_animation != NULL) {
        sLedAnimationInstance_t *animation_instance = (sLedAnimationInstance_t *) timer_arg->current_animation->data;
        
        if (animation_instance == NULL) {
            timer_arg->led_state = eWs2812bState_Idle;

            osTimerStop(timer_arg->timer);
            osMutexRelease(timer_arg->mutex);
            
            return;
        }
        
        animation_instance->build_animation(animation_instance->context);
        
        timer_arg->current_animation = timer_arg->current_animation->next;
    }

    if (!WS2812B_API_Update(timer_arg->device)) {
        
        timer_arg->led_state = eWs2812bState_Idle;

        osTimerStop(timer_arg->timer);
    }

    osMutexRelease(timer_arg->mutex);

    return;
}

static bool WS2812B_API_Update (const eWs2812b_t device) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        return false;
    }

    if (g_ws2812b_api_dynamic_lut[device].led_state != eWs2812bState_Running) {
        return false;
    }

    if (osMutexAcquire(g_ws2812b_api_dynamic_lut[device].mutex, MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    // TODO: Optimize this g_ws2812b_api_dynamic_lut[device].led_count logic for performance

    g_ws2812b_api_dynamic_lut[device].led_state = eWs2812bState_Updating;

    osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);

    if (!WS2812B_Driver_Set(device, g_ws2812b_api_dynamic_lut[device].led_data, g_ws2812b_api_static_lut[device].max_led)) {
        return false;
    }

    return true;
}

static void WS2812B_API_DriverCallback (void *context, const eLedTransferState_t transfer_state) {
    if (context == NULL) {
        return;
    }
    
    sWs2812bApiDynamicDesc_t *callback_arg = (sWs2812bApiDynamicDesc_t*) context;

    if (transfer_state == eLedTransferState_Complete) { 
        osEventFlagsSet(callback_arg->flag, TRANSFER_SUCCESS_FLAG);
    }

    return;
}

static bool WS2812B_API_BuildStaticAnimation (const sLedAnimationDesc_t *static_animation_data) {
    if (static_animation_data == NULL) {
        return false;
    }
    
    if (!WS2812B_Config_IsCorrectWs2812b(static_animation_data->device)) {
        return false;
    }

    if ((static_animation_data->animation < eLedAnimation_First) || (static_animation_data->animation >= eLedAnimation_Last)) {
        return false;
    }

    if (static_animation_data->data == NULL) {
        return false;
    }

    switch (static_animation_data->animation) {
        case eLedAnimation_SolidColor: {
            sLedAnimationSolidColor_t *data = static_animation_data->data;

            sSolidAnimationData_t solid_context = {
                .device = static_animation_data->device,
                .brightness = static_animation_data->brightness,
                .rgb = data->rgb
            };
        
            sLedAnimationInstance_t animation_instance = {
                .context = &solid_context,
                .build_animation = Animation_SolidColor_Run
            };
        
            animation_instance.build_animation(animation_instance.context);
        } break;
        case eLedAnimation_SegmentFill: {
            sLedAnimationSegmentFill_t *data = static_animation_data->data;

            sSegmentFillData_t segment_context = {
                .device = static_animation_data->device,
                .brightness = static_animation_data->brightness,
                .base_rgb = data->rgb_base,
                .segment_rgb = data->rgb_segment,
                .start_led = data->segment_start_led,
                .end_led = data->segment_end_led
            };

            sLedAnimationInstance_t animation_instance = {
                .context = &segment_context,
                .build_animation = Animation_SegmentFill_Run
            };

            animation_instance.build_animation(animation_instance.context);
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

static bool WS2812B_API_QueueDynamicAnimation (const sLedAnimationDesc_t *dynamic_animation_data) {
    if (dynamic_animation_data == NULL) {
        return false;
    }

    if (!WS2812B_Config_IsCorrectWs2812b(dynamic_animation_data->device)) {
        return false;
    }

    if ((dynamic_animation_data->animation < eLedAnimation_First) || (dynamic_animation_data->animation >= eLedAnimation_Last)) {
        return false;
    }

    if (dynamic_animation_data->data == NULL) {
        return false;
    }

    sLedAnimationInstance_t *animation_instance =  Heap_API_Malloc(sizeof(sLedAnimationInstance_t));

    if (animation_instance == NULL) {
        TRACE_ERR("QueueDynamicAnimation: Malloc failed for animation instance\n");
        
        return false;
    }

    switch (dynamic_animation_data->animation) {
        case eLedAnimation_Rainbow: {
            sLedAnimationRainbow_t *data = dynamic_animation_data->data;

            sLedRainbow_t *rainbow_context = Heap_API_Malloc(sizeof(sLedRainbow_t));
            sLedAnimationRainbow_t *rainbow_data = Heap_API_Malloc(sizeof(sLedAnimationRainbow_t));

            if ((rainbow_context == NULL) || (rainbow_data == NULL)) {
                TRACE_ERR("QueueDynamicAnimation: Malloc failed for rainbow context or data\n");
                
                return false;
            }

            rainbow_context->device = dynamic_animation_data->device;
            rainbow_context->brightness = dynamic_animation_data->brightness;
            rainbow_context->state = eRainbowState_Init;

            rainbow_data->direction = data->direction;
            rainbow_data->start_hsv_color = data->start_hsv_color;
            rainbow_data->segment_start_led = data->segment_start_led;
            rainbow_data->segment_end_led = data->segment_end_led;
            rainbow_data->speed = data->speed;
            rainbow_data->hue_step = data->hue_step;
            rainbow_data->frames_per_update = data->frames_per_update;

            rainbow_context->parameters = rainbow_data;

            animation_instance->context = rainbow_context;
            animation_instance->build_animation = Animation_Rainbow_Run;
            animation_instance->free_animation = Animation_Rainbow_Free;
        } break;
        default: {
            return false;
        } break;
    }

    animation_instance->build_animation(animation_instance->context);

    sWs2812bSequence_t *new_animation = Heap_API_Malloc(sizeof(sWs2812bSequence_t));
    
    if (new_animation == NULL) {
        TRACE_ERR("QueueDynamicAnimation: Malloc failed for new animation sequence\n");
        
        return false;
    }

    new_animation->animation = dynamic_animation_data->animation;
    new_animation->data = animation_instance;
    new_animation->next = NULL;

    if (g_ws2812b_api_dynamic_lut[dynamic_animation_data->device].current_animation != NULL) {
        new_animation->next = g_ws2812b_api_dynamic_lut[dynamic_animation_data->device].current_animation;
    } 

    g_ws2812b_api_dynamic_lut[dynamic_animation_data->device].dynamic_animations = new_animation;
    g_ws2812b_api_dynamic_lut[dynamic_animation_data->device].current_animation = new_animation;

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool WS2812B_API_InitAll (void) {
    if (g_ws2812b_api_is_init) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    if (!Timer_Driver_InitAllTimers()) {
        return false;
    }

    if (!PWM_Driver_InitAllDevices()) {
        return false;
    }

    g_ws2812b_api_is_init = true;

    for (eWs2812b_t device = eWs2812b_First; device < eWs2812b_Last; device++) {
        const sWs2812bControlDesc_t *desc = WS2812B_Config_GetWs2812bControlDesc(device);

        if (desc == NULL) {
            TRACE_ERR("Init: No control desc for device %d\n", device);

            g_ws2812b_api_is_init = false;

            return false;
        }

        g_ws2812b_api_static_lut[device] = *desc;
        
        if (!WS2812B_Driver_Init(device, &WS2812B_API_DriverCallback, &g_ws2812b_api_dynamic_lut[device])) {
            g_ws2812b_api_is_init = false;
        }

        g_ws2812b_api_dynamic_lut[device].led_data = Heap_API_Calloc(g_ws2812b_api_static_lut[device].max_led * LED_DATA_CHANNELS, sizeof(uint8_t));

        if (g_ws2812b_api_dynamic_lut[device].led_data == NULL) {
            g_ws2812b_api_is_init = false;
        }
        
        g_ws2812b_api_dynamic_lut[device].timer = osTimerNew(WS2812B_API_TimerCallback, osTimerPeriodic, &g_ws2812b_api_dynamic_lut[device], &g_ws2812b_api_static_lut[device].timer_attributes);

        if (g_ws2812b_api_dynamic_lut[device].timer == NULL) {
            g_ws2812b_api_is_init = false;
        }

        g_ws2812b_api_dynamic_lut[device].mutex = osMutexNew(&g_ws2812b_api_static_lut[device].mutex_attributes);

        if (g_ws2812b_api_dynamic_lut[device].mutex == NULL) {
            g_ws2812b_api_is_init = false;
        }

        g_ws2812b_api_dynamic_lut[device].flag = osEventFlagsNew(&g_ws2812b_api_static_lut[device].flag_attributes);

        if (g_ws2812b_api_dynamic_lut[device].flag == NULL) {
            g_ws2812b_api_is_init = false;
        }

        g_ws2812b_api_dynamic_lut[device].device = device;
    }

    return g_ws2812b_api_is_init;
}

bool WS2812B_API_AddAnimation (sLedAnimationDesc_t *animation_data) {
    if (animation_data == NULL) {
        TRACE_ERR("AddAnimation: No animation data\n");
        
        return false;
    }

    if (!WS2812B_Config_IsCorrectWs2812b(animation_data->device)) {
        TRACE_ERR("AddAnimation: Incorrect device %d\n", animation_data->device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("AddAnimation: Device not initialized\n");
        
        return false;
    }

    if (g_ws2812b_api_dynamic_lut[animation_data->device].led_state != eWs2812bState_Idle) {
        TRACE_ERR("AddAnimation: Device state not idle: [%d]\n", g_ws2812b_api_dynamic_lut[animation_data->device].led_state);
        
        return false;
    }

    if (osMutexAcquire(g_ws2812b_api_dynamic_lut[animation_data->device].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("AddAnimation: Failed to acquire mutex for device %d\n", animation_data->device);
        
        return false;
    }

    g_ws2812b_api_dynamic_lut[animation_data->device].led_state = eWs2812bState_Building;

    osMutexRelease(g_ws2812b_api_dynamic_lut[animation_data->device].mutex);

    bool is_execute_successful = true;

    switch (animation_data->animation) {
        case eLedAnimation_SolidColor: {
            if (!WS2812B_API_BuildStaticAnimation(animation_data)) {
                TRACE_ERR("AddAnimation: Build static animation [%d] failed\n", animation_data->animation);
                
                is_execute_successful = false;
            }
        } break;
        case eLedAnimation_SegmentFill: {
            if (!WS2812B_API_BuildStaticAnimation(animation_data)) {
                TRACE_ERR("AddAnimation: Build static animation [%d] failed\n", animation_data->animation);
                
                is_execute_successful = false;
            }
        } break;
        case eLedAnimation_Rainbow: {
            if (!WS2812B_API_QueueDynamicAnimation(animation_data)) {
                TRACE_ERR("AddAnimation: Build dynamic animation [%d] failed\n", animation_data->animation);

                is_execute_successful = false;
            }
        } break; 
        default: {
            TRACE_ERR("AddAnimation: Animation not supported [%d]\n", animation_data->animation);

            is_execute_successful = false;
        } break;       
    }

    if (osMutexAcquire(g_ws2812b_api_dynamic_lut[animation_data->device].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("AddAnimation: Failed to acquire mutex for state reset\n");
        
        is_execute_successful = false;
    }

    g_ws2812b_api_dynamic_lut[animation_data->device].led_state = eWs2812bState_Idle;

    osMutexRelease(g_ws2812b_api_dynamic_lut[animation_data->device].mutex);

    return is_execute_successful;
}

bool WS2812B_API_ClearAnimations (const eWs2812b_t device) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("ClearAnimations: Incorrect device %d\n", device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("ClearAnimations: Device not initialized\n");

        return false;
    }

    if (g_ws2812b_api_dynamic_lut[device].led_state != eWs2812bState_Idle) {
        TRACE_ERR("ClearAnimations: Device state not idle: [%d]\n", g_ws2812b_api_dynamic_lut[device].led_state);

        return false;
    }

    if (g_ws2812b_api_dynamic_lut[device].dynamic_animations == NULL) {
        return true;
    }

    if (osMutexAcquire(g_ws2812b_api_dynamic_lut[device].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("ClearAnimations: Failed to acquire mutex for device %d\n", device);
        
        return false;
    }

    while (g_ws2812b_api_dynamic_lut[device].dynamic_animations != NULL) {
        sWs2812bSequence_t *sequence = g_ws2812b_api_dynamic_lut[device].dynamic_animations;
        sLedAnimationInstance_t *instance = (sLedAnimationInstance_t *) sequence->data;

        if (instance == NULL) {
            TRACE_ERR("ClearAnimations: No animation instance\n");
            
            osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);
            
            return false;
        }

        if (instance->free_animation != NULL && instance->context != NULL) {
            instance->free_animation(instance->context);
        }
        
        g_ws2812b_api_dynamic_lut[device].dynamic_animations = sequence->next;

        if (!WS2812B_API_FreeData(sequence)) {
            TRACE_ERR("ClearAnimations: Heap API failed\n");
            
            osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);
            
            return false;
        }
    }

    memset(g_ws2812b_api_dynamic_lut[device].led_data, 0, g_ws2812b_api_static_lut[device].max_led * LED_DATA_CHANNELS);

    osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);

    return true;
}

bool WS2812B_API_Start (const eWs2812b_t device) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("Start: Incorrect device %d\n", device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("Start: Device not initialized\n");
        
        return false;
    }

    if (g_ws2812b_api_dynamic_lut[device].led_state != eWs2812bState_Idle) {
        TRACE_ERR("Start: Device state not idle: [%d]\n", g_ws2812b_api_dynamic_lut[device].led_state);

        return false;
    }

    if (osMutexAcquire(g_ws2812b_api_dynamic_lut[device].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Start: Failed to acquire mutex for device %d\n", device);
        
        return false;
    }
    
    g_ws2812b_api_dynamic_lut[device].led_state = eWs2812bState_Running;

    osEventFlagsClear(g_ws2812b_api_dynamic_lut[device].flag, TRANSFER_SUCCESS_FLAG);

    if (!WS2812B_API_Update(device)) {
        TRACE_ERR("Start: Update failed for device %d\n", device);
        
        osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);
        
        return false;
    }

    uint32_t flag = osEventFlagsWait(g_ws2812b_api_dynamic_lut[device].flag, TRANSFER_SUCCESS_FLAG, osFlagsWaitAny | osFlagsNoClear, DEFAULT_FLAG_TIMEOUT);

    osEventFlagsClear(g_ws2812b_api_dynamic_lut[device].flag, TRANSFER_SUCCESS_FLAG);

    if (flag != TRANSFER_SUCCESS_FLAG) {
        TRACE_ERR("Start: Received incorrect flag: [%ld]\n", (int32_t) flag);

        g_ws2812b_api_dynamic_lut[device].led_state = eWs2812bState_Idle;

        osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);

        return false;
    }

    if (g_ws2812b_api_dynamic_lut[device].dynamic_animations == NULL) {
        g_ws2812b_api_dynamic_lut[device].led_state = eWs2812bState_Idle;

        osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);

        return true;
    }

    osTimerStart(g_ws2812b_api_dynamic_lut[device].timer, REFRESH_RATE);
    osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);

    return true;
}

bool WS2812B_API_Stop (const eWs2812b_t device) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("Stop: Incorrect device %d\n", device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("Stop: Device not initialized\n");

        return false;
    }

    if (!osTimerIsRunning(g_ws2812b_api_dynamic_lut[device].timer)) {
        return true;
    }

    if (osMutexAcquire(g_ws2812b_api_dynamic_lut[device].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Stop: Failed to acquire mutex for device %d\n", device);
        
        return false;
    }

    osTimerStop(g_ws2812b_api_dynamic_lut[device].timer);

    g_ws2812b_api_dynamic_lut[device].led_state = eWs2812bState_Idle;

    osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);

    uint32_t flag = osEventFlagsWait(g_ws2812b_api_dynamic_lut[device].flag, TRANSFER_SUCCESS_FLAG, osFlagsWaitAny, DEFAULT_FLAG_TIMEOUT);

    if (flag != TRANSFER_SUCCESS_FLAG) {
        TRACE_ERR("Stop: Received incorrect flag: [%ld]\n", (int32_t) flag);

        return false;
    }

    return true;
}

bool WS2812B_API_Reset (const eWs2812b_t device) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("Reset: Incorrect device %d\n", device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("Reset: Device not initialized\n");

        return false;
    }

    if (g_ws2812b_api_dynamic_lut[device].led_state != eWs2812bState_Idle) {
        TRACE_ERR("Reset: Device state not idle: [%d]\n", g_ws2812b_api_dynamic_lut[device].led_state);

        return false;
    }

    if (!WS2812B_Driver_Reset(device)) {
        TRACE_ERR("Reset: Driver reset failed for device %d\n", device);
        
        return false;
    }

    uint32_t flag = osEventFlagsWait(g_ws2812b_api_dynamic_lut[device].flag, TRANSFER_SUCCESS_FLAG, osFlagsWaitAny, DEFAULT_FLAG_TIMEOUT);

    if (flag != TRANSFER_SUCCESS_FLAG) {
        TRACE_ERR("Reset: Received incorrect flag: [%ld]\n", (int32_t) flag);

        return false;
    }

    if (osMutexAcquire(g_ws2812b_api_dynamic_lut[device].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Reset: Failed to acquire mutex for device %d\n", device);
        
        return false;
    }

    memset(g_ws2812b_api_dynamic_lut[device].led_data, 0, g_ws2812b_api_static_lut[device].max_led * LED_DATA_CHANNELS);

    g_ws2812b_api_dynamic_lut[device].led_state = eWs2812bState_Idle;

    osMutexRelease(g_ws2812b_api_dynamic_lut[device].mutex);

    return true;
}

bool WS2812B_API_FreeData (void *data) {
    if (data == NULL) {
        TRACE_ERR("FreeData: No data to free\n");
        
        return false;
    }

    return Heap_API_Free(data);
}

uint32_t WS2812B_API_GetLedCount (const eWs2812b_t device) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("GetLedCount: Incorrect device %d\n", device);
        
        return 0;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("GetLedCount: Device not initialized\n");

        return 0;
    }

    return g_ws2812b_api_static_lut[device].max_led;
}

bool WS2812B_API_SetColor (const eWs2812b_t device, size_t led_number, const uint8_t red, const uint8_t green, const uint8_t blue) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("SetColor: Incorrect device %d\n", device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("SetColor: Device not initialized\n");

        return false;
    }

    if (led_number >= g_ws2812b_api_static_lut[device].max_led) {
        TRACE_ERR("SetColor: Led number %d is out of range\n", led_number);
        
        return false;
    }

    led_number *= LED_DATA_CHANNELS;

    g_ws2812b_api_dynamic_lut[device].led_data[led_number] = red;
    g_ws2812b_api_dynamic_lut[device].led_data[led_number + 1] = green;
    g_ws2812b_api_dynamic_lut[device].led_data[led_number + 2] = blue;

    return true;
}

bool WS2812B_API_FillColor (const eWs2812b_t device, const uint8_t red, const uint8_t green, const uint8_t blue) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("FillColor: Incorrect device %d\n", device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("FillColor: Device not initialized\n");

        return false;
    }

    size_t led_byte = 0;

    for (size_t led = 0; led < g_ws2812b_api_static_lut[device].max_led; led++) {
        led_byte = led * LED_DATA_CHANNELS;
        g_ws2812b_api_dynamic_lut[device].led_data[led_byte] = red;
        g_ws2812b_api_dynamic_lut[device].led_data[led_byte + 1] = green;
        g_ws2812b_api_dynamic_lut[device].led_data[led_byte + 2] = blue;
    }

    return true;
}

bool WS2812B_API_FillSegment (const eWs2812b_t device, const size_t start_led, const size_t end_led, const uint8_t red, const uint8_t green, const uint8_t blue) {
    if (!WS2812B_Config_IsCorrectWs2812b(device)) {
        TRACE_ERR("FillSegment: Incorrect device %d\n", device);
        
        return false;
    }

    if (!g_ws2812b_api_is_init) {
        TRACE_ERR("FillSegment: Device not initialized\n");

        return false;
    }

    if (start_led >= end_led || end_led > g_ws2812b_api_static_lut[device].max_led) {
        TRACE_ERR("FillSegment: Incorrect segment range; start: %d, end: %d\n", start_led, end_led);
        
        return false;
    }

    size_t led_byte = 0;

    for (size_t led = start_led; led < end_led; led++) {
        led_byte = led * LED_DATA_CHANNELS;
        g_ws2812b_api_dynamic_lut[device].led_data[led_byte] = red;
        g_ws2812b_api_dynamic_lut[device].led_data[led_byte + 1] = green;
        g_ws2812b_api_dynamic_lut[device].led_data[led_byte + 2] = blue;
    }

    return true;
}

#endif
