#ifndef SOURCE_API_LED_API_H_
#define SOURCE_API_LED_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_LED) || defined(ENABLE_PWM_LED)
#include <stdbool.h>
#include <stdint.h>
#include "led_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool LED_API_Init (void);

#if defined(ENABLE_LED)
bool LED_API_TurnOn (const eLed_t led);
bool LED_API_TurnOff (const eLed_t led);
bool LED_API_Toggle (const eLed_t led);
bool LED_API_Blink (const eLed_t led, const size_t blink_time, const uint16_t blink_frequency);
bool LED_API_IsCorrectBlinkTime (const size_t blink_time);
bool LED_API_IsCorrectBlinkFrequency (const uint16_t blink_frequency);
#endif /* ENABLE_LED */

#if defined(ENABLE_PWM_LED)
bool LED_API_Set_Brightness (const eLedPwm_t led, const uint16_t brightness);
bool LED_API_Pulse (const eLedPwm_t led, const size_t pulsing_time, const uint16_t pulse_frequency);
bool LED_API_IsCorrectDutyCycle (const eLedPwm_t led, const uint16_t duty_cycle);
bool LED_API_IsCorrectPulseTime (const size_t pulse_time);
bool LED_API_IsCorrectPulseFrequency (const uint16_t pulse_frequency);
#endif /* ENABLE_PWM_LED */

#endif /* defined(ENABLE_LED) || defined(ENABLE_PWM_LED) */
#endif /* SOURCE_API_LED_API_H_ */
