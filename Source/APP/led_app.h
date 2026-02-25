#ifndef SOURCE_APP_LED_APP_H_
#define SOURCE_APP_LED_APP_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_LED) || defined(ENABLE_PWM_LED)
#include <stdbool.h>
#include <stdint.h>
#include "led_api.h"
#include "led_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eLedTask {
    eLedTask_First = 0,
    
    #if defined(ENABLE_LED)
    eLedTask_Set,
    eLedTask_Reset,
    eLedTask_Toggle,
    eLedTask_Blink,
    #endif /* ENABLE_LED */

    #if defined(ENABLE_PWM_LED)
    eLedTask_Set_Brightness,
    eLedTask_Pulse,
    #endif /* ENABLE_PWM_LED */
    
    eLedTask_Last
} eLedTask_t;

typedef struct sLedCommandDesc {
    eLedTask_t task;
    void *data;
} sLedCommandDesc_t;

typedef struct sLedCommon {
    eLed_t led;
} sLedCommon_t;

#if defined(ENABLE_LED)
typedef struct sLedBlink {
    eLed_t led;
    size_t blink_time;
    uint16_t blink_frequency;
} sLedBlink_t;
#endif /* ENABLE_LED */

#if defined(ENABLE_PWM_LED)
typedef struct sLedSetBrightness {
    eLedPwm_t led;
    uint16_t duty_cycle;
} sLedSetBrightness_t;

typedef struct sLedPulse {
    eLedPwm_t led;
    size_t pulse_time;
    uint16_t pulse_frequency;
} sLedPulse_t;
#endif /* ENABLE_PWM_LED */
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool LED_APP_Init (void);
bool LED_APP_AddTask (sLedCommandDesc_t *task_to_message_queue);

#endif /* defined(ENABLE_LED) || defined(ENABLE_PWM_LED) */
#endif /* SOURCE_APP_LED_APP_H_ */
