#ifndef SOURCE_APP_MOTOR_APP_H_
#define SOURCE_APP_MOTOR_APP_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_MOTOR
#include <stdbool.h>
#include <stdint.h>
#include "motor_api.h"
#include "motor_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eMotorTask {
    eMotorTask_First,
    eMotorTask_Set = eMotorTask_First,
    eMotorTask_Stop,
    #ifdef ENABLE_PID_CONTROL
    eMotorTask_SetRpm,
    #endif /* ENABLE_PID_CONTROL */
    eMotorTask_Last
} eMotorTask_t;

typedef struct sMotorCommandDesc {
    eMotorTask_t task;
    void *data;
} sMotorCommandDesc_t;

typedef struct sMotorSet {
    float speed;
    eMotorDirection_t direction;
    eMotorControl_t mode;
} sMotorSet_t;

#ifdef ENABLE_PID_CONTROL
typedef struct sMotorSetRpm {
    eMotor_t motor;
    float target_rpm;
    eMotorControl_t mode;
} sMotorSetRpm_t;
#endif /* ENABLE_PID_CONTROL */
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Motor_APP_Init (void);
bool Motor_APP_Add_Task (sMotorCommandDesc_t *task_to_message_queue);

#endif /* ENABLE_MOTOR */
#endif /* SOURCE_APP_MOTOR_APP_H_ */
