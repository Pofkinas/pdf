#ifndef SOURCE_DRIVER_MOTOR_DRIVER_H_
#define SOURCE_DRIVER_MOTOR_DRIVER_H_
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

#define STOP_SPEED 0

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eMotorDriver {
    eMotorDriver_First = 0,

    #ifdef USE_MOTOR_A
    eMotorDriver_A,
    #endif

    #ifdef USE_MOTOR_B
    eMotorDriver_B,
    #endif

    eMotorDriver_Last
} eMotorDriver_t;

typedef enum eMotorRotation {
    eMotorRotation_First,
    eMotorRotation_Forward = eMotorRotation_First,
    eMotorRotation_Backward,
    eMotorRotation_Last
} eMotorRotation_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Motor_Driver_DefinePerips (const sMotorConstDesc_t *motor_lut);
bool Motor_Driver_InitAllMotors (void);
bool Motor_Driver_EnableMotor (const eMotorDriver_t motor);
bool Motor_Driver_DisableMotor (const eMotorDriver_t motor);
bool Motor_Driver_SetSpeed (const eMotorDriver_t motor, const eMotorRotation_t rotation_dir, const size_t speed);
bool Motor_Driver_GetMaxSpeed (const eMotorDriver_t motor, uint16_t *speed);

#endif /* SOURCE_DRIVER_MOTOR_DRIVER_H_ */
