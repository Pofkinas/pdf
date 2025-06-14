#ifndef SOURCE_DRIVER_GPIO_DRIVER_H_
#define SOURCE_DRIVER_GPIO_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "framework_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eGpioPin {
    eGpioPin_First = 0,

    #ifdef USE_UART_DEBUG
    eGpioPin_DebugTx,
    eGpioPin_DebugRx,
    #endif

    #ifdef USE_ONBOARD_LED
    eGpioPin_OnboardLed,
    #endif

    #ifdef USE_PULSE_LED
    eGpioPin_PulseLed,
    #endif

    #ifdef USE_START_BUTTON
    eGpioPin_StartButton,
    #endif

    #ifdef USE_UART_UROS_TX
    eGpioPin_uRosTx,
    #endif

    #ifdef USE_MOTOR_A
    eGpioPin_MotorA_A1,
    eGpioPin_MotorA_A2,
    #endif

    #ifdef USE_MOTOR_B
    eGpioPin_MotorB_A1,
    eGpioPin_MotorB_A2,
    #endif

    #ifdef USE_TCRT5000_RIGHT
    eGpioPin_Tcrt5000_Right,
    #endif

    #ifdef USE_TCRT5000_LEFT
    eGpioPin_Tcrt5000_Left,
    #endif

    #ifdef USE_WS2812B_1
    eGpioPin_Ws2812B_1,
    #endif

    #ifdef USE_WS2812B_2
    eGpioPin_Ws2812B_2,
    #endif

    #ifdef USE_I2C1
    eGpioPin_I2c1_SCL,
    eGpioPin_I2c1_SDA,
    #endif

    #ifdef USE_VL53L0_XSHUT1
    eGpioPin_vl53l0_Xshut_1,
    #endif

    #ifdef USE_VL53L0_XSHUT2
    eGpioPin_vl53l0_Xshut_2,
    #endif

    eGpioPin_Last
} eGpioPin_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void GPIO_Driver_DefinePins (const sGpioDesc_t *gpio_lut);
bool GPIO_Driver_InitAllPins (void);
bool GPIO_Driver_WritePin (const eGpioPin_t gpio_pin, const bool pin_state);
bool GPIO_Driver_ReadPin (const eGpioPin_t gpio_pin, bool *pin_state);
bool GPIO_Driver_TogglePin (const eGpioPin_t gpio_pin);
bool GPIO_Driver_ResetPin (const eGpioPin_t gpio_pin);

#endif /* SOURCE_DRIVER_GPIO_DRIVER_H_ */
