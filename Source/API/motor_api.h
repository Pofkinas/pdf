#ifndef SOURCE_API_MOTOR_API_H_
#define SOURCE_API_MOTOR_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_MOTOR
#include <stdbool.h>
#include <stdint.h>
#include "motor_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eMotorControl {
    eMotorControl_First = 0,
    eMotorControl_None = eMotorControl_First,
    eMotorControl_Ramp,
    eMotorControl_Last
} eMotorControl_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Motor_API_Init (void);
bool Motor_API_SetMotors (const float speed, const eMotorDirection_t direction, const eMotorControl_t control);
bool Motor_API_SetMotorSpeed (const eMotor_t motor, const float speed, const eMotorDirection_t direction, const eMotorControl_t control);
bool Motor_API_StopAllMotors (void);
bool Motor_API_EnableAllMotors (void);
bool Motor_API_DisableAllMotors (void);

bool Motor_API_IsCorrectSpeed (const float speed);
bool Motor_API_IsCorrectMode (const eMotorControl_t mode);
bool Motor_API_IsMotorEnabled (const eMotor_t motor);
bool Motor_API_GetMotorRotation (const eMotor_t motor, eMotorRotation_t *rotation);

#endif /* ENABLE_MOTOR */
#endif /* SOURCE_API_MOTOR_API_H_ */
