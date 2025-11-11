/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sMotorDriverDesc_t g_static_motor_driver_lut[eMotor_Last] = {
    [eMotor_A] = {
        .timer = eTimer_TIM3,
        .fwd_channel = ePwm_MotorA_A2,
        .rev_channel = ePwm_MotorA_A1
    }
};

const static sMotor_t g_static_motor_lut[eMotor_Last] = {
    [eMotor_A] = {
        .mutex_attributes = {.name = "Motor_A_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .timer_attributes = {.name = "Motor_A_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0U},
        .motor_speed_offset = MOTOR_A_SPEED_OFFSET,
        .position = eMotorPosition_Right,
        .rotation = {
            [eMotorDirection_Forward] = eMotorRotation_Forward,
            [eMotorDirection_Reverse] = eMotorRotation_Backward,
            [eMotorDirection_Right] =  eMotorRotation_Backward,
            [eMotorDirection_RightSoft] = eMotorRotation_Forward,
            [eMotorDirection_Left] = eMotorRotation_Forward,
            [eMotorDirection_LeftSoft] = eMotorRotation_Forward
        }
    }
}; 
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

const osThreadAttr_t g_motor_thread_attributes = {
    .name = "Motor_APP_Thread",
    .stack_size = 128 * 5,
    .priority = (osPriority_t) osPriorityNormal
};

const osMessageQueueAttr_t g_motor_message_queue_attributes = {
    .name = "Motor_Command_MessageQueue", 
    .attr_bits = 0, 
    .cb_mem = NULL, 
    .cb_size = 0, 
    .mq_mem = NULL, 
    .mq_size = 0
};

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Motor_Config_IsCorrectMotor (const eMotor_t motor) {
    return (motor >= eMotor_First) && (motor < eMotor_Last);
}

bool Motor_Config_IsCorrectDirection (const eMotorDirection_t direction) {
    return (direction >= eMotorDirection_First) && (direction < eMotorDirection_Last);
}

const sMotorDriverDesc_t *Motor_Config_GetMotorDriverDesc (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return NULL;
    }

    return &g_static_motor_driver_lut[motor];
}

const sMotor_t *Motor_Config_GetMotorDesc (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return NULL;
    }

    return &g_static_motor_lut[motor];
}