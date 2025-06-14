/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_driver.h"

#ifdef USE_MOTOR

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sMotorDynamic {
    bool is_enabled;
    uint16_t max_speed;
    uint16_t current_speed;
} sMotorDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sMotorConstDesc_t g_motor_lut[eMotorDriver_Last];
static bool g_is_all_motor_init = false;

/* clang-format off */
static sMotorDynamic_t g_dynamic_motor_lut[eMotorDriver_Last] = {
    #ifdef USE_MOTOR_A
    [eMotorDriver_A] = {
        .is_enabled = false,
        .max_speed = 0,
        .current_speed = 0
    },
    #endif

    #ifdef USE_MOTOR_B
    [eMotorDriver_B] = {
        .is_enabled = false,
        .max_speed = 0,
        .current_speed = 0
    }
    #endif
};
/* clang-format on */

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

void Motor_Driver_DefinePerips (const sMotorConstDesc_t *motor_lut) {
    if (motor_lut == NULL) {
        return;
    }

    g_motor_lut = motor_lut;

    g_is_all_motor_init = false;

    return;
}

bool Motor_Driver_InitAllMotors (void) {
    if (g_is_all_motor_init) {
        return true;
    }

    if (eMotorDriver_Last == 1) {
        return false;
    }

    for (eMotorDriver_t motor = (eMotorDriver_First + 1); motor < eMotorDriver_Last; motor++) {
        g_dynamic_motor_lut[motor].max_speed = Timer_Driver_GetResolution(g_motor_lut[motor].timer);
    }

    g_is_all_motor_init = true;

    return g_is_all_motor_init;
}

bool Motor_Driver_EnableMotor (const eMotorDriver_t motor) {
    if ((motor <= eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
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

bool Motor_Driver_DisableMotor (const eMotorDriver_t motor) {
    if ((motor <= eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
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

bool Motor_Driver_SetSpeed (const eMotorDriver_t motor, const eMotorRotation_t rotation_dir, const size_t speed) {
    if ((motor <= eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
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

bool Motor_Driver_GetMaxSpeed (const eMotorDriver_t motor, uint16_t *speed) {
    if ((motor <= eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
        return false;
    }

    if (speed == NULL) {
        return false;
    }

    *speed = g_dynamic_motor_lut[motor].max_speed;

    return true;
}

#endif
