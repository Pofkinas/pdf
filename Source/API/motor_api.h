#ifndef SOURCE_API_MOTOR_API_H_
#define SOURCE_API_MOTOR_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_MOTOR
#include <stdbool.h>
#include <stddef.h>
#include "motor_config.h"

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

bool Motor_API_Init (void);
bool Motor_API_SetSpeed (const size_t speed, const eMotorDirection_t direction);
bool Motor_API_StopAllMotors (void);
bool Motor_API_EnableAllMotors (void);
bool Motor_API_DisableAllMotors (void);
bool Motor_API_IsCorrectDirection (const eMotorDirection_t direction);
bool Motor_API_IsCorrectSpeed (const size_t speed);
bool Motor_API_IsMotorEnabled (const eMotor_t motor);

#endif /* ENABLE_MOTOR */
#endif /* SOURCE_API_MOTOR_API_H_ */
