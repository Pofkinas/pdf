#ifndef CONFIG_LED_CONFIG_H_
#define CONFIG_LED_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "framework_config.h"

#ifdef ENABLE_LED
#include "gpio_config.h"
#endif

#ifdef ENABLE_PWM_LED
#include "pwm_config.h"
#endif

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef struct sLedDesc {
    eGpioPin_t led_pin;
    bool is_inverted;
    osTimerAttr_t blink_timer_attributes;
    osMutexAttr_t blink_mutex_attributes;
} sLedDesc_t;

typedef struct sLedPwmDesc {
    ePwmDevice_t pwm_device;
    osTimerAttr_t pulse_timer_attributes;
    osMutexAttr_t pulse_mutex_attributes;
} sLedPwmDesc_t;

typedef enum eLed {
    eLed_First = 0,

    #ifdef ENABLE_LED
    eLed_OnboardLed,
    #endif

    eLed_Last
} eLed_t;

typedef enum eLedPwm {
    eLedPwm_First = 0,

    #ifdef ENABLE_PWM_LED
    eLedPwm_PulseLed,
    #endif

    eLedPwm_Last
} eLedPwm_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

#if defined(ENABLE_LED)
const sLedDesc_t *LED_Config_GetLedDesc (const eLed_t led);
#endif

#if defined(ENABLE_PWM_LED)
const sLedPwmDesc_t *LED_Config_GetPwmLedDesc (const eLedPwm_t led);
#endif

#endif /* CONFIG_LED_CONFIG_H_ */
