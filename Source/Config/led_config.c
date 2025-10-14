/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "led_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
#ifdef ENABLE_LED
const static sLedDesc_t g_led_static_lut[eLed_Last] = {
    #ifdef USE_ONBOARD_LED
    [eLed_OnboardLed] = {
        .led_pin = eGpioPin_OnboardLed,
        .is_inverted = true,
        .blink_timer_attributes = {.name = "Onboard_LED_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .blink_mutex_attributes = {.name = "Onboard_LED_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
    }
    #endif
};
#endif

#ifdef ENABLE_PWM_LED
const static sLedPwmDesc_t g_pwm_led_static_lut[eLedPwm_Last] = {
    #ifdef ENABLE_PWM_LED
    [eLedPwm_PulseLed] = {
        .pwm_device = ePwmDevice_PulseLed,
        .pulse_timer_attributes = {.name = "Pulse_LED_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .pulse_mutex_attributes = {.name = "Pulse_LED_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
    }
    #endif
};
#endif
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

#ifdef ENABLE_LED
const sLedDesc_t *LED_Config_GetLedDesc (const eLed_t led) {
    if ((led <= eLed_First) || (led >= eLed_Last)) {
        return NULL;
    }

    return &g_led_static_lut[led];
}
#endif

#ifdef ENABLE_PWM_LED
const sLedPwmDesc_t *LED_Config_GetPwmLedDesc (const eLedPwm_t led) {
    if ((led <= eLedPwm_First) || (led >= eLedPwm_Last)) {
        return NULL;
    }

    return &g_pwm_led_static_lut[led];
}
#endif
