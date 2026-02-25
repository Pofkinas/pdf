#ifndef SOURCE_DRIVER_PWM_DRIVER_H_
#define SOURCE_DRIVER_PWM_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_PWM)
#include <stdbool.h>
#include <stdint.h>
#include "pwm_config.h"

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

bool PWM_Driver_InitAllDevices (void);
bool PWM_Driver_EnableDevice (const ePwm_t device);
bool PWM_Driver_DisableDevice (const ePwm_t device);
bool PWM_Driver_ChangeDutyCycle (const ePwm_t device, const uint32_t value);
uint32_t PWM_Driver_GetRegAddr (const ePwm_t device);
uint16_t PWM_Driver_GetDeviceTimerResolution (const ePwm_t device);
uint32_t PWM_Driver_GetCompareValue (const ePwm_t device);

#endif /* ENABLE_PWM */
#endif /* SOURCE_DRIVER_PWM_DRIVER_H_ */
