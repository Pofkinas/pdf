/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_driver.h"

#ifdef ENABLE_MOTOR
#include "pwm_driver.h"
#include "timer_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sMotorDynamic {
    bool is_enabled;
    size_t max_speed;
    size_t current_speed;
} sMotorDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_all_motor_init = false;
static sMotorDriverDesc_t g_motor_lut[eMotor_Last] = {0};
static sMotorDynamic_t g_dynamic_motor_lut[eMotor_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Motor_Driver_InitAllMotors (void) {
    if (g_is_all_motor_init) {
        return true;
    }

    g_is_all_motor_init = true;

    for (eMotor_t motor = (eMotor_First + 1); motor < eMotor_Last; motor++) {
        const sMotorDriverDesc_t *desc = Motor_Config_GetMotorDriverDesc(motor);

        if (desc == NULL) {
            g_is_all_motor_init = false;
            return false;
        }

        g_motor_lut[motor] = *desc;
        
        g_dynamic_motor_lut[motor].current_speed = STOP_SPEED;
        g_dynamic_motor_lut[motor].max_speed = Timer_Driver_GetResolution(g_motor_lut[motor].timer);
    }

    return g_is_all_motor_init;
}

bool Motor_Driver_EnableMotor (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return false;
    }

    if (!g_is_all_motor_init) {
        return false;
    }

    if (g_dynamic_motor_lut[motor].is_enabled) {
        return true;
    }

    if (!PWM_Driver_Enable_Device(g_motor_lut[motor].fwd_channel)) {
        return false;
    }

    if (!PWM_Driver_Enable_Device(g_motor_lut[motor].rev_channel)) {
        return false;
    }

    g_dynamic_motor_lut[motor].is_enabled = true;

    return g_dynamic_motor_lut[motor].is_enabled;
}

bool Motor_Driver_DisableMotor (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return false;
    }

    if (!g_is_all_motor_init) {
        return false;
    }

    if (!g_dynamic_motor_lut[motor].is_enabled) {
        return true;
    }

    if (!PWM_Driver_Disable_Device(g_motor_lut[motor].fwd_channel)) {
        return false;
    }

    if (!PWM_Driver_Disable_Device(g_motor_lut[motor].rev_channel)) {
        return false;
    }

    g_dynamic_motor_lut[motor].is_enabled = false;

    return g_dynamic_motor_lut[motor].is_enabled;
}

bool Motor_Driver_SetSpeed (const eMotor_t motor, const eMotorRotation_t rotation_dir, const size_t speed) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return false;
    }

    if (!g_is_all_motor_init) {
        return false;
    }

    if ((rotation_dir < eMotorRotation_First) || (rotation_dir >= eMotorRotation_Last)) {
        return false;
    }

    if (speed > g_dynamic_motor_lut[motor].max_speed) {
        return false;
    }

    if (!g_dynamic_motor_lut[motor].is_enabled) {
        return false;
    }

    switch (rotation_dir) {
        case eMotorRotation_Forward: {
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].rev_channel, STOP_SPEED)) {
                return false;
            }
            
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].fwd_channel, speed)) {
                return false;
            }
        } break;
        case eMotorRotation_Backward: {
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].fwd_channel, STOP_SPEED)) {
                return false;
            }
            
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].rev_channel, speed)) {
                return false;
            }
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

bool Motor_Driver_GetMaxSpeed (const eMotor_t motor, size_t *speed) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return false;
    }

    if (speed == NULL) {
        return false;
    }

    if (!g_is_all_motor_init) {
        return false;
    }

    *speed = g_dynamic_motor_lut[motor].max_speed;

    return true;
}

#endif /* ENABLE_MOTOR */
