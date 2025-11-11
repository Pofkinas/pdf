#ifndef CONFIG_MOTOR_CONFIG_H_
#define CONFIG_MOTOR_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "pwm_config.h"
#include "timer_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define MOTOR_A_SPEED_OFFSET 0
#define MOTOR_B_SPEED_OFFSET 0

#define STOP_SPEED 0
#define DEFAULT_MOTOR_SPEED 60

#define INPUT_MIN_SPEED 0
#define INPUT_MAX_SPEED 100
#define DEFAULT_SOFT_START_STEP 1

/// Speed scaling limits (%)
#define MIN_SCALED_SPEED 0
#define MAX_SCALED_SPEED 100
#define SOFT_TURN_SPEED_OFFSET 20

/// Soft-start configuration
#define MOTOR_SOFT_START_STEPS 30
#define MOTOR_SOFT_START_TIMER_MS 5

#define MOTOR_MESSAGE_QUEUE_CAPACITY 10
#define MOTOR_MESSAGE_QUEUE_PRIORITY 0U
#define MOTOR_MESSAGE_QUEUE_TIMEOUT 0U

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eMotor {
    eMotor_First = 0,
    eMotor_A = eMotor_First,
    eMotor_B,
    eMotor_Last
} eMotor_t;

typedef enum eMotorPosition {
    eMotorPosition_First = 0,
    eMotorPosition_Right = eMotorPosition_First,
    eMotorPosition_Left,
    eMotorPosition_Center,
    eMotorPosition_Last
} eMotorPosition_t;

typedef enum eMotorRotation {
    eMotorRotation_First = 0,
    eMotorRotation_Forward = eMotorRotation_First,
    eMotorRotation_Backward,
    eMotorRotation_Last
} eMotorRotation_t;

typedef enum eMotorDirection {
    eMotorDirection_First = 0,
    eMotorDirection_Forward = eMotorDirection_First,
    eMotorDirection_Reverse,
    eMotorDirection_Right,
    eMotorDirection_RightSoft,
    eMotorDirection_Left,
    eMotorDirection_LeftSoft,
    eMotorDirection_Last
} eMotorDirection_t;

typedef struct sMotorDriverDesc {
    eTimer_t timer;
    ePwm_t fwd_channel;
    ePwm_t rev_channel;
} sMotorDriverDesc_t;

typedef struct sMotor {
    osMutexAttr_t mutex_attributes;
    osTimerAttr_t timer_attributes;
    uint8_t motor_speed_offset;
    eMotorPosition_t position;
    eMotorRotation_t rotation[eMotorDirection_Last];
} sMotor_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern const osThreadAttr_t g_motor_thread_attributes;
extern const osMessageQueueAttr_t g_motor_message_queue_attributes;

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Motor_Config_IsCorrectMotor (const eMotor_t motor);
bool Motor_Config_IsCorrectDirection (const eMotorDirection_t direction);
const sMotorDriverDesc_t *Motor_Config_GetMotorDriverDesc (const eMotor_t motor);
const sMotor_t *Motor_Config_GetMotorDesc (const eMotor_t motor);

#endif /* CONFIG_MOTOR_CONFIG_H_ */
