/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_driver.h"

#ifdef ENABLE_MOTOR
#include "pwm_driver.h"
#include "timer_driver.h"
#ifdef USE_TB6612FNG
#include "gpio_driver.h"
#endif /* USE_TB6612FNG */

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

    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        const sMotorDriverDesc_t *desc = Motor_Config_GetMotorDriverDesc(motor);

        if (desc == NULL) {
            g_is_all_motor_init = false;
            return false;
        }

        g_motor_lut[motor] = *desc;
        
        g_dynamic_motor_lut[motor].current_speed = STOP_SPEED;
        g_dynamic_motor_lut[motor].max_speed = Timer_Driver_GetResolution(g_motor_lut[motor].timer);

        #ifdef USE_TB6612FNG
        if (!GPIO_Driver_WritePin(g_motor_lut[motor].in1, false)) {
            g_is_all_motor_init = false;
            return false;
        }
        if (!GPIO_Driver_WritePin(g_motor_lut[motor].in2, false)) {
            g_is_all_motor_init = false;
            return false;
        }
        #endif /* USE_TB6612FNG */
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

    if (!PWM_Driver_Enable_Device(g_motor_lut[motor].pwm_1)) {
        return false;
    }

    #ifdef USE_MX1508
    if (!PWM_Driver_Enable_Device(g_motor_lut[motor].pwm_2)) {
        return false;
    }
    #endif /* USE_MX1508 */

    g_dynamic_motor_lut[motor].is_enabled = true;

    return true;
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

    if (!PWM_Driver_Disable_Device(g_motor_lut[motor].pwm_1)) {
        return false;
    }

    #ifdef USE_MX1508
    if (!PWM_Driver_Disable_Device(g_motor_lut[motor].pwm_2)) {
        return false;
    }
    #endif /* USE_MX1508 */

    #ifdef USE_TB6612FNG
    if (!GPIO_Driver_WritePin(g_motor_lut[motor].in1, false)) {
        return false;
    }
    if (!GPIO_Driver_WritePin(g_motor_lut[motor].in2, false)) {
        return false;
    }
    #endif /* USE_TB6612FNG */

    g_dynamic_motor_lut[motor].is_enabled = false;

    return true;
}

bool Motor_Driver_SetSpeed (const eMotor_t motor, const eMotorRotation_t rotation_dir, const uint16_t speed) {
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
        case eMotorRotation_CW: {
            #ifdef USE_MX1508
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_2, STOP_SPEED)) {
                return false;
            }
            #endif /* USE_MX1508 */
            
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_1, speed)) {
                return false;
            }

            #ifdef USE_TB6612FNG
            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in1, true)) {
                return false;
            }
            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in2, false)) {
                return false;
            }
            #endif /* USE_TB6612FNG */
        } break;
        case eMotorRotation_CCW: {
            #ifdef USE_MX1508
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_1, STOP_SPEED)) {
                return false;
            }
            
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_2, speed)) {
                return false;
            }
            #endif /* USE_MX1508 */

            #ifdef USE_TB6612FNG
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_1, speed)) {
                return false;
            }

            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in1, false)) {
                return false;
            }

            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in2, true)) {
                return false;
            }
            #endif /* USE_TB6612FNG */
        } break;
        case eMotorRotation_Stop: {
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_1, STOP_SPEED)) {
                return false;
            }

            #ifdef USE_MX1508
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_2, STOP_SPEED)) {
                return false;
            }
            #endif /* USE_MX1508 */

            #ifdef USE_TB6612FNG
            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in1, false)) {
                return false;
            }

            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in2, false)) {
                return false;
            }
            #endif /* USE_TB6612FNG */
        } break;
        case eMotorRotation_Brake: {
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_1, speed)) {
                return false;
            }

            #ifdef USE_MX1508
            if (!PWM_Driver_Change_Duty_Cycle(g_motor_lut[motor].pwm_2, speed)) {
                return false;
            }
            #endif /* USE_MX1508 */

            #ifdef USE_TB6612FNG
            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in1, true)) {
                return false;
            }

            if (!GPIO_Driver_WritePin(g_motor_lut[motor].in2, true)) {
                return false;
            }
            #endif /* USE_TB6612FNG */
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

bool Motor_Driver_GetMaxSpeed (const eMotor_t motor, uint16_t *speed) {
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
