#ifndef CONFIG_PROJECT_CONFIG_H_
#define CONFIG_PROJECT_CONFIG_H_

//==============================================================================
// FEATURE FLAGS
//------------------------------------------------------------------------------
/// Uncomment the flags for the peripherals and modules you need:

/// -- DEBUG
#define USE_UART_DEBUG                              // Enable debug messages

/// -- CLI
#define USE_CLI                                // Enable Command Line Interface (CLI) support

/// -- LEDs
#define USE_LED                           // Enable on-board LED

//==============================================================================
// SYSTEM TIMING
//------------------------------------------------------------------------------
/// System clock frequency (Hz)

#define SYSTEM_CLOCK_HZ 100000000UL

//==============================================================================
// UART CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_UART_DEBUG) || defined(USE_UART_DEBUG)
#define ENABLE_UART
#define UART_2 eUartDriver_Debug
#endif

#ifdef USE_UART_DEBUG
#define UART_DEBUG_BUFFER_CAPACITY 256
#endif

#ifdef USE_UART_UROS_TX
#define UART_UROS_BUFFER_CAPACITY 64
#endif

//==============================================================================
// LED CONFIGURATION
//------------------------------------------------------------------------------

#if defined(USE_LED)
#define ENABLE_LED
/// Blink Maximum time (s)
#define MAX_BLINK_TIME 59
/// Blink frequency limits (Hz)
#define MIN_BLINK_FREQUENCY 2
#define MAX_BLINK_FREQUENCY 100
#endif

#if defined(USE_PULSE_LED)
#define ENABLE_PWM_LED
// Pulsing Maximum time (s)
#define MAX_PULSING_TIME 59
// Pulsing frequency limits (Hz)
#define MAX_PULSE_FREQUENCY 500
#endif

//==============================================================================
// MISCELLANEOUS
//------------------------------------------------------------------------------

#if defined(USE_MOTOR) || defined(USE_WS2812B) || defined(USE_PWM_LED)
#define USE_PWM
#endif

#endif /* CONFIG_PROJECT_CONFIG_H_ */
