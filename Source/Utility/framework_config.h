#ifndef SOURCE_UTILITY_FRAMEWORK_CONFIG_H_
#define SOURCE_UTILITY_FRAMEWORK_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#ifndef PROJECT_CONFIG_H
#include "example_config.h"
#error "PROJECT_CONFIG_H not defined. Add -DPROJECT_CONFIG_H=\"project_config.h\" to your build."
#endif /* PROJECT_CONFIG_H */

#include PROJECT_CONFIG_H
#define SYSTEM_MS_TICS (SYSTEM_CLOCK_HZ / 1000)

/**********************************************************************************************************************
 * Project configuration analysis
 *********************************************************************************************************************/

#if (defined(ENABLE_UART) || defined(ENABLE_PWM) || defined(ENABLE_LED) || defined(ENABLE_PWM_LED)\
    || defined(ENABLE_IO) || defined(ENABLE_EXTI) || defined(ENABLE_I2C) || defined(ENABLE_WS2812B)\
    || defined(ENABLE_VL53L0X) || defined(ENABLE_MOTOR) || defined(ENABLE_LCD)) && !defined(ENABLE_GPIO)
#error "At least one peripheral or module requires GPIO to be enabled."
#endif /* (ENABLE_UART || ENABLE_PWM || ENABLE_LED || ENABLE_PWM_LED || ENABLE_IO || ENABLE_EXTI ||\
         ENABLE_I2C || ENABLE_WS2812B || ENABLE_VL53L0X || ENABLE_MOTOR || ENABLE_LCD) && !ENABLE_GPIO */

#if defined(ENABLE_UART_DEBUG) && !defined(ENABLE_UART)
#error "DEBUG_UART requires UART to be enabled."
#endif /* ENABLE_UART_DEBUG && !ENABLE_UART */

#if defined(ENABLE_CLI) && (!defined(DEBUG_UART) || !defined(ENABLE_UART_DEBUG))
#error "CLI requires DEBUG_UART to be enabled."
#endif /* ENABLE_CLI && (!DEBUG_UART || !ENABLE_UART_DEBUG) */

#if defined(ENABLE_CLI) && (!defined(ENABLE_DEFAULT_CMD) || !defined(ENABLE_CUSTOM_CMD))
#error "CLI requires DEFAULT_CMD or CUSTOM_CMD to be enabled."
#endif /* ENABLE_CLI && (!ENABLE_DEFAULT_CMD || !ENABLE_CUSTOM_CMD) */

#if defined(ENABLE_WS2812B) && !defined(ENABLE_DMA)
#error "WS2812B requires DMA to be enabled."
#endif /* ENABLE_WS2812B && !ENABLE_DMA */

#if defined(ENABLE_VL53L0X) && !defined(ENABLE_I2C)
#error "VL53L0X requires I2C to be enabled."
#endif /* ENABLE_VL53L0X && !ENABLE_I2C */

#if defined(ENABLE_LCD) && !defined(ENABLE_I2C)
#error "LCD requires I2C to be enabled."
#endif /* ENABLE_LCD && !ENABLE_I2C */

#if defined(ENABLE_MOTOR) && (!defined(ENABLE_PWM) || !defined(ENABLE_TIMER))
#error "MOTOR requires PWM and TIMER to be enabled."
#endif /* ENABLE_MOTOR && (!ENABLE_PWM || !ENABLE_TIMER) */

#if defined(ENABLE_PWM_LED) && (!defined(ENABLE_PWM) || !defined(ENABLE_TIMER))
#error "PWM_LED requires PWM and TIMER to be enabled."
#endif /* ENABLE_PWM_LED && (!ENABLE_PWM || !ENABLE_TIMER) */

#if defined(ENABLE_PWM) && !defined(ENABLE_TIMER)
#error "PWM requires TIMER to be enabled."
#endif /* ENABLE_PWM && !ENABLE_TIMER */

#if defined(ENABLE_CUSTOM_CLI) && !defined(ENABLE_CLI)
#error "INCLUDE_CUSTOM_CLI requires ENABLE_CLI to be defined."
#endif /* INCLUDE_CUSTOM_CLI && !ENABLE_CLI */

#if defined(ENABLE_LED_ANIMATION) && !defined(ENABLE_WS2812B)
#error "ENABLE_LED_ANIMATION requires ENABLE_WS2812B to be defined."
#endif /* ENABLE_LED_ANIMATION && !ENABLE_WS2812B */

#if defined(ENABLE_LED_ANIMATION) && !defined(ENABLE_COLOUR)
#error "ENABLE_LED_ANIMATION requires ENABLE_COLOUR to be defined."
#endif /* ENABLE_LED_ANIMATION && !ENABLE_COLOUR */

#endif /* SOURCE_UTILITY_FRAMEWORK_CONFIG_H_ */
