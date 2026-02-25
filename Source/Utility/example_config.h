#ifndef SOURCE_UTILITY_EXAMPLE_CONFIG_H_
#define SOURCE_UTILITY_EXAMPLE_CONFIG_H_

/******************************************************************************
 * @file 
 * @brief Project example configuration header file for the
 * Pofkinas Development Framework (PDF).
 *
 * This file contains example feature flags and configuration settings
 * for various modules and peripherals used in the application.
 *
 * @note This file should be used as a template for your project configuration
 * configuration file `project_config.h` in config directory
 * (e.g. project_name/config/).
 * Peripherals may be configured in their respective *_config.{c,h} files.
 * See `examples` branch for implementation examples.
 *****************************************************************************/

//=============================================================================
// FEATURE FLAGS
//-----------------------------------------------------------------------------
/// Uncomment the flags for the peripherals and modules you need:

/// -- GPIO                    // Enable GPIO functionality
#define ENABLE_GPIO

/// -- TIMER                   // Enable TIMER functionality
#define ENABLE_TIMER

/// -- UART                    // Enable UART functionality
#define ENABLE_UART

/// -- PWM                     // Enable PWM functionality
#define ENABLE_PWM

/// -- DEBUG UART              // Enable Debug UART functionality
#define ENABLE_UART_DEBUG                   

/// -- CLI                     // Enable Command Line Interface (CLI) over UART
#define ENABLE_CLI
#define ENABLE_DEFAULT_CMD
#define ENABLE_CUSTOM_CMD

/// -- LEDs                    // Enable LED functionality
#define ENABLE_LED
#define ENABLE_PWM_LED

/// -- I/Os                    // Enable Input/Output functionality
#define ENABLE_IO

/// -- EXTI                    // Enable EXTI functionality
#define ENABLE_EXTI

/// -- DMA                     // Enable DMA functionality
#define ENABLE_DMA

/// -- I²C bus                 // Enable I2C functionality
#define ENABLE_I2C

/// -- WS2812B LED strips      // Enable WS2812B LED strip functionality
#define ENABLE_WS2812B

/// -- LED animation           // Enable LED animation functionality
#define ENABLE_LED_ANIMATION

/// -- Time-of-flight sensors  // Enable VL53L0X sensor
#define ENABLE_VL53L0X         

/// -- Motors                  // Enable Motor functionality
#define ENABLE_MOTOR

/// -- LCD                     // Enable LCD functionality
#define ENABLE_LCD

//=============================================================================
// SYSTEM CONFIGURATION
//-----------------------------------------------------------------------------
/// System clock frequency (Hz)

#define SYSTEM_CLOCK_HZ 100000000UL

//=============================================================================
// UART CONFIGURATION
//-----------------------------------------------------------------------------

#if defined(ENABLE_UART)
// #define UART1 
// #define UART_1_BAUDRATE

#define UART2 eUart_Debug
#define UART_2_BAUDRATE eBaudrate_115200

#if defined(ENABLE_UART_DEBUG)
#define DEBUG_UART UART2

#define DEBUG_DELIMITER "\r\n"
#define DEBUG_MESSAGE_SIZE 256
#define UART_DEBUG_BUFFER_CAPACITY 256

#define DEBUG_MESSAGE_TIMEOUT 1000
#define DEBUG_MUTEX_TIMEOUT 0U
#endif /* ENABLE_UART_DEBUG */
#endif /* ENABLE_UART */

//=============================================================================
// LED CONFIGURATION
//-----------------------------------------------------------------------------

#if defined(ENABLE_LED)
/// Blink Maximum time (s)
#define MAX_BLINK_TIME 59
/// Blink frequency limits (Hz)
#define MIN_BLINK_FREQUENCY 2
#define MAX_BLINK_FREQUENCY 100
#endif /* ENABLE_LED */

#if defined(ENABLE_PWM_LED)
// Pulsing Maximum time (s)
#define MAX_PULSING_TIME 59
// Pulsing frequency limits (Hz)
#define MAX_PULSE_FREQUENCY 500
#define MAX_DUTY_CYCLE 65535
#define MIN_PULSE_FREQUENCY (MAX_DUTY_CYCLE / 1000)
#endif /* ENABLE_PWM_LED */

//=============================================================================
// EXTI CONFIGURATION
//-----------------------------------------------------------------------------

#if defined(ENABLE_EXTI)
#define EXTI0 eExti_StartButton
// #define EXTI1
// #define EXTI2
// #define EXTI3
// #define EXTI4
// #define EXTI9_5
// #define EXTI15_10
#endif /* ENABLE_EXTI */

//=============================================================================
// I²C BUS CONFIGURATION
//-----------------------------------------------------------------------------

#if defined(ENABLE_I2C)
#define I2C_1 eI2c_1
// #define I2C_2

#define I2C_MAX_DATA_SIZE 16

#if defined(I2C_1)
#define I2C_1_CLOCK_SPEED 100000U

#define LCD_1_ADDRESS 0x27
#define VL53L0X_1_I2C_ADDRESS 0x62
#endif /* I2C_1 */

#if defined(I2C_2)
#endif /* I2C_2 */
#endif /* ENABLE_I2C */

//=============================================================================
// DMA CONFIGURATION
//-----------------------------------------------------------------------------

#if defined(ENABLE_DMA)
// #define DMA_1_STREAM_0
// #define DMA_1_STREAM_1
// #define DMA_1_STREAM_2
// #define DMA_1_STREAM_3
#define DMA_1_STREAM_4 eDma_Ws2812b_1
// #define DMA_1_STREAM_5
// #define DMA_1_STREAM_6
// #define DMA_1_STREAM_7

// #define DMA_2_STREAM_0
// #define DMA_2_STREAM_1
// #define DMA_2_STREAM_2
// #define DMA_2_STREAM_3
// #define DMA_2_STREAM_4
// #define DMA_2_STREAM_5
// #define DMA_2_STREAM_6
// #define DMA_2_STREAM_7
#endif /* ENABLE_DMA */

//=============================================================================
// VL53L0X CONFIGURATION
//-----------------------------------------------------------------------------

#if defined(ENABLE_VL53L0X)
#define VL53L0X_I2C_PHERIPH I2C_1
#endif /* ENABLE_VL53L0X */

//=============================================================================
// DEBUG
//-----------------------------------------------------------------------------

#if defined(ENABLE_UART_DEBUG)
// Custom debug flags
#define DEBUG_MAIN
#define CUSTOM_CLI_CMD_HANDLERS

#define DEBUG_CLI_APP
#define DEBUG_DEFAULT_CMD
#define DEBUG_LED_APP
#define DEBUG_MOTOR_APP

// API layer debug flags
// #define DEBUG_CMD_API
// #define DEBUG_CMD_API_HELPER
// #define DEBUG_DEFAULT_CMD
// #define DEBUG_UART_API
// #define DEBUG_I2C_API
// #define DEBUG_IO_API
// #define DEBUG_LCD_API
// #define DEBUG_LED_API
// #define DEBUG_MOTOR_API
// #define DEBUG_VL53L0XV2_API
// #define DEBUG_WS2812B_API
#endif /* ENABLE_UART_DEBUG */

//=============================================================================
// MISCELLANEOUS
//-----------------------------------------------------------------------------

#if defined(ENABLE_CLI)
#define CLI_APP_THREAD_STACK_SIZE (256 * 6)
#define CLI_APP_THREAD_PRIORITY osPriorityNormal

#define CLI_COMMAND_MESSAGE_CAPACITY 20
#define RESPONSE_MESSAGE_CAPACITY 128
#define CMD_SEPARATOR ","
#endif /* ENABLE_CLI */

#define HEAP_API_MUTEX_TIMEOUT 0U

#define BYTE 8
#define BASE_10 10

#endif /* SOURCE_UTILITY_EXAMPLE_CONFIG_H_ */
