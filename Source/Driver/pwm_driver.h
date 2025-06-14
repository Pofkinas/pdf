#ifndef SOURCE_DRIVER_PWM_DRIVER_H_
#define SOURCE_DRIVER_PWM_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "framework_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum ePwmDevice {
    ePwmDevice_First = 0,

    #ifdef USE_PULSE_LED
    ePwmDevice_PulseLed,
    #endif

    #ifdef USE_MOTOR_A
    ePwmDevice_MotorA_A1,
    ePwmDevice_MotorA_A2,
    #endif

    #ifdef USE_MOTOR_B
    ePwmDevice_MotorB_A1,
    ePwmDevice_MotorB_A2,
    #endif

    #ifdef USE_WS2812B_1
    ePwmDevice_Ws2812b_1,
    #endif

    #ifdef USE_WS2812B_2
    ePwmDevice_Ws2812b_2,
    #endif

    ePwmDevice_Last
} ePwmDevice_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void PWM_Driver_DefinePerips (const sPwmOcChannelDesc_t *pwm_lut);
bool PWM_Driver_InitAllDevices (void);
bool PWM_Driver_Enable_Device (const ePwmDevice_t device);
bool PWM_Driver_Disable_Device (const ePwmDevice_t device);
bool PWM_Driver_Change_Duty_Cycle (const ePwmDevice_t device, const size_t value);
uint32_t PWM_Driver_GetRegAddr (const ePwmDevice_t device);
uint16_t PWM_Driver_GetDeviceTimerResolution (const ePwmDevice_t device);
uint32_t PWM_Driver_GetCompareValue (const ePwmDevice_t device);

#endif /* SOURCE_DRIVER_PWM_DRIVER_H_ */
