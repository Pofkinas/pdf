#ifndef FRAMEWORK_UTILITY_EXAMPLE_CONFIG_H_
#define FRAMEWORK_UTILITY_EXAMPLE_CONFIG_H_

/***********************************************************************************************************************
 * @file 
 * @brief Platform exmaple configuration header file for the Pofkinas Development Framework (PDF).
 *
 * This file contains example feature flags and configuration settings for various modules and peripherals used in the application.
 *
 * @note This file should be used as a template for your platform configuration for file `project_config.h`in main directory (e.g. ProjectName/Application/).
 * @note Make sure to change `#include platform_init.h` in `project_config.h`to your used platfrom.
 ***********************************************************************************************************************/

#include "platform_init.h"

//==============================================================================
// FEATURE FLAGS
//------------------------------------------------------------------------------
/// Uncomment the flags for the peripherals and modules you need:

/// -- UART / Debug console
#define USE_UART_DEBUG                            // Enable debug UART interface
#define USE_UART_UROS_TX                          // Enable uROS UART transport

/// -- DEBUG
#define ENABLE_DEBUG                              // Enable debug messages

/// -- I²C bus
#define USE_I2C1                                  // Enable I2C1 peripheral
#define USE_I2C2                                  // Enable I2C2 peripheral

/// -- DMA                                        
#define USE_DMA                                   // Enable DMA support

/// -- CLI
#define ENABLE_CLI                                // Enable Command Line Interface (CLI) support
#define INCLUDE_PROJECT_CLI                       // Include custom CLI commands from project_cli_lut.h

/// -- LEDs
#define USE_ONBOARD_LED                           // Enable on-board LED
#define USE_PULSE_LED                             // Enable PWM controlled LEDs

/// -- I/Os
#define USE_START_BUTTON                          // Enable External Start button
#define USE_TCRT5000_LEFT                         // Enable reflectance sensor
#define USE_TCRT5000_RIGHT                        // Enable reflectance sensor

#define USE_EXTI0                                 // Enable EXTI0 interrupt
#define USE_EXTI1                                 // Enable EXTI1 interrupt
#define USE_EXTI2                                 // Enable EXTI2 interrupt
#define USE_EXTI3                                 // Enable EXTI3 interrupt
#define USE_EXTI4                                 // Enable EXTI4 interrupt
#define USE_EXTI9_5                               // Enable EXTI9_5 interrupt
#define USE_EXTI15_10                             // Enable EXTI15_10 interrupt

/// -- WS2812B LED strips
#define USE_WS2812B_1                             // Enable LED strip
#define USE_WS2812B_2                             // Enable LED strip

/// -- Time-of-flight sensors
#define USE_VL53L0X_1                             // Enable VL53L0X sensor
#define USE_VL53L0_XSHUT1                         // Enable VL53L0X sensor XSHUT
#define USE_VL53L0X_2                             // Enable VL53L0X sensor
#define USE_VL53L0_XSHUT2                         // Enable VL53L0X sensor XSHUT

/// -- Motors
#define USE_MOTOR_A                               // Enable Motor A
#define USE_MOTOR_B                               // Enable Motor B

/// -- LCD
#define USE_LCD_1                                 // Enable LCD 1 (I2C) interface

//==============================================================================
// SYSTEM TIMING
//------------------------------------------------------------------------------
/// System clock frequency (Hz)

#define SYSTEM_CLOCK_HZ 100000000UL

//==============================================================================
// UART CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_UART_DEBUG) || defined(USE_UART_UROS_TX)
#define USE_UART
#define UART_1 eUartDriver_uRos
#define UART_2 eUartDriver_Debug
#endif

#ifdef USE_UART_DEBUG
#define UART_DEBUG_BUFFER_CAPACITY 256
#endif

#ifdef USE_UART_UROS_TX
#define UART_UROS_BUFFER_CAPACITY 64
#endif

//==============================================================================
// I2C BUS CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_I2C1) || defined(USE_I2C2)
#define USE_I2C
#endif

#ifdef USE_I2C1
#define I2C_1 eI2cDriver_I2c1
#define I2C_MAX_DATA_SIZE 16
#endif

#ifdef USE_I2C2
#define I2C_2 eI2cDriver_I2c2
#define I2C_MAX_DATA_SIZE 16
#endif

//==============================================================================
// LED CONFIGURATION
//------------------------------------------------------------------------------

#ifdef USE_ONBOARD_LED
/// Invert LED logic (true = active-low)
#define USE_ONBOARD_LED_INVERTED false
#endif

#if defined(USE_ONBOARD_LED)
#define USE_LED
/// Blink Maximum time (s)
#define MAX_BLINK_TIME 59
/// Blink frequency limits (Hz)
#define MIN_BLINK_FREQUENCY 2
#define MAX_BLINK_FREQUENCY 100
#endif

#if defined(USE_PULSE_LED)
#define USE_PWM_LED
// Pulsing Maximum time (s)
#define MAX_PULSING_TIME 59
// Pulsing frequency limits (Hz)
#define MAX_PULSE_FREQUENCY 500
#endif

//==============================================================================
// I/O CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_START_BUTTON) || defined(USE_TCRT5000_LEFT) || defined(USE_TCRT5000_RIGHT)
#define USE_IO
#endif

#ifdef USE_START_BUTTON
/// Start button active state
#define START_BUTTON_ACTIVE_STATE eActiveState_Low
/// Debounce settings 
#define START_BUTTON_ENABLE_DEBOUNCE true
#if START_BUTTON_ENABLE_DEBOUNCE == true
#define STARTSTOP_BUTTON_DEBOUNCE_PERIOD 50U
#endif
/// Enable EXTI interrupt for button
#define START_BUTTON_EXTI true
/// Button triggered event
#define STARTSTOP_TRIGGERED_EVENT 0x01U
#endif

#ifdef USE_TCRT5000_RIGHT
#define TCRT_RIGHT_ACTIVE_STATE eActiveState_Both
#define TCRT_RIGHT_ENABLE_DEBOUNCE true
#define TCRT_RIGHT_EXTI true
#define TCRT5000_RIGHT_TRIGGERED_EVENT 0x01U
#endif

#ifdef USE_TCRT5000_LEFT
#define TCRT_LEFT_ACTIVE_STATE eActiveState_Both
#define TCRT_LEFT_ENABLE_DEBOUNCE true
#define TCRT_LEFT_EXTI true
#define TCRT5000_LEFT_TRIGGERED_EVENT 0x02U
#endif

#if defined(USE_EXTI0) || defined(USE_EXTI1) || defined(USE_EXTI2) || defined(USE_EXTI3) || \
    defined(USE_EXTI4) || defined(USE_EXTI9_5) || defined(USE_EXTI15_10)
#define USE_EXTI
#endif

#if defined(TCRT_RIGHT_ENABLE_DEBOUNCE) || defined(TCRT_LEFT_ENABLE_DEBOUNCE)
/// Used to debounce PWM EMS
#define TCRT5000_DEBOUNCE_PERIOD 15U 
#endif

//==============================================================================
// WS2812B LED STRIPS CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_WS2812B_1) || defined(USE_WS2812B_2)
#define USE_WS2812B
#endif

#ifdef USE_WS2812B_1
/// Number of LEDs on strip
#define WS2812B_1_LED_COUNT 0
#endif

#ifdef USE_WS2812B_2
/// Number of LEDs on strip
#define WS2812B_2_LED_COUNT 0
#endif

//==============================================================================
// VL53L0x TIME-OF-FLIGHT CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_VL53L0X_1) || defined(USE_VL53L0X_2)
#define USE_VL53L0X
/// Stop timeout for sensor #1 (ms)
#define DEFAULT_STOP_TIMEOUT 100
/// Calibration distances (mm)
#define DEFAULT_OFFSET_CALIB_DISTANCE_MM 100
#define DEFAULT_CROSSTALK_CALIB_DISTANCE_MM 200
#endif

#ifdef USE_VL53L0X_1
#define RANGING_PROFILE_VL53L0_1 eVl53l0xRangeProfile_LongRange
#endif

#ifdef USE_VL53L0X_2
#define RANGING_PROFILE_VL53L0_2 eVl53l0xRangeProfile_LongRange
#endif

//==============================================================================
// MOTOR CONFIGURATION
//------------------------------------------------------------------------------

/// Motor Speed range 0% ÷ 100%
#if defined(USE_MOTOR_A) || defined(USE_MOTOR_B)
#define USE_MOTOR
#define MOTOR_RIGHT_SPEED_OFFSET 0
#define MOTOR_LEFT_SPEED_OFFSET 0

/// Speed scaling limits (%)
#define MIN_SCALED_SPEED 0
#define MAX_SCALED_SPEED 100
#define SOFT_TURN_SPEED_OFFSET 20

/// Soft-start configuration
#define MOTOR_SOFT_START_STEPS 30
#define MOTOR_SOFT_START_TIMER_MS 5

/// Default run speed (%)
#define DEFAULT_MOTOR_SPEED 60
#endif

//==============================================================================
// CLI SETTINGS
//------------------------------------------------------------------------------

#define CLI_COMMAND_MESSAGE_CAPACITY 20
#define RESPONSE_MESSAGE_CAPACITY 128

//==============================================================================
// LCD CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_LCD_1)
#define USE_LCD
#endif

//==============================================================================
// MISCELLANEOUS
//------------------------------------------------------------------------------

#if defined(USE_MOTOR) || defined(USE_WS2812B) || defined(USE_PWM_LED)
#define USE_PWM
#endif

#endif /* FRAMEWORK_UTILITY_EXAMPLE_CONFIG_H_ */
