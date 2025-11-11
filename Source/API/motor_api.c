/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_api.h"

#ifdef ENABLE_MOTOR
#include <stdint.h>
#include <math.h>
#include "cmsis_os2.h"
#include "debug_api.h"
#include "motor_driver.h"
#include "pwm_driver.h"
#include "timer_driver.h"
#include "gpio_driver.h"
#include "float_parts.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define MUTEX_TIMEOUT 0U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sMotorDynamic {
    eMotor_t motor;
    bool is_enabled;
    uint16_t speed;
    uint16_t target_speed;
    uint16_t max_speed;
    eMotorDirection_t direction;
    eMotorDirection_t target_direction;
    osMutexId_t mutex;
    osTimerId_t timer;
    int16_t step_value;
    eMotorControl_t mode;
    bool is_braking;
} sMotorDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_MOTOR_API
CREATE_MODULE_NAME (MOTOR_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_MOTOR_API */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_all_motors_init = false;

static sMotor_t g_static_motor_lut[eMotor_Last] = {0}; 
static sMotorDynamic_t g_dynamic_motor_lut[eMotor_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static uint16_t Motor_API_Scale_Speed (const eMotor_t motor, const float speed);
static void Motor_API_TimerCallback (void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static uint16_t Motor_API_Scale_Speed (const eMotor_t motor, const float speed) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        TRACE_ERR("Motor_API_Scale_Speed: Incorrect motor type [%d]\n", motor);
        
        return 0;
    }

    if (!Motor_API_IsCorrectSpeed(speed)) {
        TRACE_ERR("Motor_API_Scale_Speed: Incorrect speed [%ld.%03u]\n", FLOAT_INTEGER_PART(speed), FLOAT_FRACTIONAL_PART(speed, 3));
        
        return 0;
    }
    
    if (speed <= INPUT_MIN_SPEED) {
        return 0;
    }

    if (speed >= INPUT_MAX_SPEED) {
        return (uint16_t) lrint((MAX_SCALED_SPEED * g_dynamic_motor_lut[motor].max_speed / INPUT_MAX_SPEED));
    }

    float scaled_speed = MIN_SCALED_SPEED + ((speed * (MAX_SCALED_SPEED - MIN_SCALED_SPEED)) / INPUT_MAX_SPEED);

    return (uint16_t) lrint((scaled_speed * g_dynamic_motor_lut[motor].max_speed / INPUT_MAX_SPEED));
}

static void Motor_API_TimerCallback (void *arg) {
    sMotorDynamic_t *motor_desc = (sMotorDynamic_t*) arg;

    if (motor_desc == NULL) {
        return;
    }

    if (eMotorDirection_Brake == motor_desc->direction) {
        osTimerStop(motor_desc->timer);
        motor_desc->speed = STOP_SPEED;
        
        if (eMotorControl_Ramp == motor_desc->mode) {
            if (STOP_SPEED != motor_desc->target_speed) {
                motor_desc->step_value = (motor_desc->target_speed - motor_desc->speed) / MOTOR_RAMP_STEPS;

                osTimerStart(motor_desc->timer, MOTOR_RAMP_TIMER_MS);
            }
        } else {
            motor_desc->speed = motor_desc->target_speed;
        }

        motor_desc->direction = motor_desc->target_direction;
        Motor_Driver_SetSpeed(motor_desc->motor, g_static_motor_lut[motor_desc->motor].rotation[motor_desc->direction], motor_desc->speed);
        
        return;
    }

    switch (motor_desc->mode) {
        case eMotorControl_Ramp: {
            uint16_t speed = motor_desc->speed + motor_desc->step_value;

            if (abs(speed - motor_desc->target_speed) >= MOTOR_RAMP_SPEED_THRESHOLD) {
                speed = motor_desc->target_speed;

                osTimerStop(motor_desc->timer);
            }

            motor_desc->speed = speed;
            Motor_Driver_SetSpeed(motor_desc->motor, g_static_motor_lut[motor_desc->motor].rotation[motor_desc->direction], speed);
        } break;
        default: {
            osTimerStop(motor_desc->timer);
        } break;
    }

    return;
}

static void Motor_API_Statup_TimerCallback (void *arg) {
    sMotorDynamic_t *motor_desc = (sMotorDynamic_t*) arg;

    uint16_t speed = motor_desc->step_value * motor_desc->soft_start_step;

    Motor_API_SetMotorSpeed(motor_desc->motor, speed, motor_desc->direction);

    motor_desc->soft_start_step++;

    if (motor_desc->soft_start_step >= MOTOR_SOFT_START_STEPS) {
        if (motor_desc->speed > speed) {
            Motor_API_SetMotorSpeed(motor_desc->motor, motor_desc->speed, motor_desc->direction);
        }

        motor_desc->soft_start_step = DEFAULT_SOFT_START_STEP;
        motor_desc->is_soft_start_running = false;

        osTimerStop(motor_desc->soft_start_timer);
    }

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Motor_API_Init (void) {
    if (g_is_all_motors_init) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        TRACE_ERR("Init: Failed to initialize GPIO pins\n");
        
        return false;
    }

    if (!Timer_Driver_InitAllTimers()) {
        TRACE_ERR("Init: Failed to initialize timers\n");

        return false;
    }

    if (!PWM_Driver_InitAllDevices()) {
        TRACE_ERR("Init: Failed to initialize PWM devices\n");
        
        return false;
    }

    if (!Motor_Driver_InitAllMotors()) {
        TRACE_ERR("Init: Failed to initialize motor drivers\n");
        
        return false;
    }

    g_is_all_motors_init = true;
    
    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        const sMotor_t *desc = Motor_Config_GetMotorDesc(motor);

        if (desc == NULL) {
            TRACE_ERR("Init: No motor desc for motor [%d]\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }

        g_static_motor_lut[motor] = *desc;
        
        g_dynamic_motor_lut[motor].mutex = osMutexNew(&g_static_motor_lut[motor].mutex_attributes);

        if (g_dynamic_motor_lut[motor].mutex == NULL) {
            TRACE_ERR("Init: Failed to create mutex for motor [%d]\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }

        g_dynamic_motor_lut[motor].timer = osTimerNew(Motor_API_TimerCallback, osTimerPeriodic, &g_dynamic_motor_lut[motor], &g_static_motor_lut[motor].timer_attributes);

        if (g_dynamic_motor_lut[motor].timer == NULL) {
            TRACE_ERR("Init: Failed to create soft start timer for motor [%d]\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }
        
        if (!Motor_Driver_GetMaxSpeed(motor, &g_dynamic_motor_lut[motor].max_speed)) {
            TRACE_ERR("Init: Failed to get max speed for motor [%d]\n", motor);
            TRACE_ERR("Init: Failed to get max speed for motor %d\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }

        g_dynamic_motor_lut[motor].motor = motor;
        g_dynamic_motor_lut[motor].direction = eMotorDirection_Stop;
        g_dynamic_motor_lut[motor].mode = eMotorControl_None;
    }

    return g_is_all_motors_init;
}

bool Motor_API_SetMotors (const float speed, const eMotorDirection_t direction, const eMotorControl_t control) {
    if (!Motor_API_IsCorrectSpeed(speed)) {
        TRACE_ERR("SetMotors: Incorrect speed [%ld.%03u]\n", FLOAT_INTEGER_PART(speed), FLOAT_FRACTIONAL_PART(speed, 3));
        
        return false;
    }

    if (!Motor_Config_IsCorrectDirection(direction)) {
        TRACE_ERR("SetMotors: Incorrect direction [%d]\n", direction);
        
        return false;
    }

    if (!Motor_API_IsCorrectMode(control)) {
        TRACE_ERR("SetMotors: Incorrect control type [%d]\n", control);
        
        return false;
    }

    bool is_success = true;

    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        uint16_t target_speed = speed;

        switch (direction) {
            case eMotorDirection_Right: {
                if (g_static_motor_lut[motor].position == eMotorPosition_Left) {
                    target_speed = 0;
                }
            } break;
            case eMotorDirection_RightSoft: {
                if (g_static_motor_lut[motor].position == eMotorPosition_Left) {
                    if (target_speed > SOFT_TURN_SPEED_OFFSET) {
                        target_speed -= SOFT_TURN_SPEED_OFFSET;
                    } else {
                        TRACE_WRN("SetMotors: Speed [%u] for soft turn too small for motor [%d]\n", target_speed, motor);
                        target_speed = STOP_SPEED;
                    }
                }
            } break;
            case eMotorDirection_Left: {
                if (g_static_motor_lut[motor].position == eMotorPosition_Right) {
                    target_speed = 0;
                }
            } break;
            case eMotorDirection_LeftSoft: {
                if (g_static_motor_lut[motor].position == eMotorPosition_Right) {
                    if (target_speed > SOFT_TURN_SPEED_OFFSET) {
                        target_speed -= SOFT_TURN_SPEED_OFFSET;
                    } else {
                        TRACE_WRN("SetMotors: Speed [%u] for soft turn too small for motor [%d]\n", target_speed, motor);
                        target_speed = STOP_SPEED;
                    }
                }
            } break;
            case eMotorDirection_Stop: {
                if (target_speed != STOP_SPEED) {
                    TRACE_WRN("SetMotors: Speed [%u] for stop command, setting to 0 for motor [%d]\n", target_speed, motor);

                    target_speed = STOP_SPEED;
                }
            } break;
            default:{
                break;
            }
        }

        if (!Motor_API_SetMotorSpeed(motor, target_speed, direction, control)) { 
            is_success = false;

            continue;
        }
    }

    return is_success;
}

bool Motor_API_SetMotorSpeed (const eMotor_t motor, const float speed, const eMotorDirection_t direction, const eMotorControl_t control) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        TRACE_ERR("SetMotorSpeed: Incorrect motor [%d]\n", motor);
        
        return false;
    }

    if (!Motor_API_IsCorrectSpeed(speed)) {
        TRACE_ERR("SetMotorSpeed: Incorrect speed [%ld.%03u]\n", FLOAT_INTEGER_PART(speed), FLOAT_FRACTIONAL_PART(speed, 3));
        
        return false;
    }

    if (!Motor_Config_IsCorrectDirection(direction)) {
        TRACE_ERR("SetMotorSpeed: Incorrect direction [%d]\n", direction);
        
        return false;
    }

    if (!Motor_API_IsCorrectMode(control)) {
        TRACE_ERR("SetMotorSpeed: Incorrect control type [%d]\n", control);
        
        return false;
    }

    if (osTimerIsRunning(g_dynamic_motor_lut[motor].timer)) {
        osTimerStop(g_dynamic_motor_lut[motor].timer);
        
        TRACE_WRN("SetMotorSpeed: Motor [%d] timer running\n", motor);
    }

    if (!Motor_API_IsMotorEnabled(motor)) {
        TRACE_WRN("SetMotorSpeed: Motor [%d] is not enabled; enabling all motors...\n", motor);

        if (!Motor_API_EnableAllMotors()) {
            TRACE_ERR("SetMotorSpeed: Failed to enable all motors\n");

            return false;
        }
    }

    if (((eMotorDirection_Brake == direction) || (eMotorDirection_Stop == direction)) && (STOP_SPEED != speed)) {
        TRACE_ERR("SetMotorSpeed: Speed [%ld.%03u] != 0, for motor [%d] brake/stop command\n", FLOAT_INTEGER_PART(speed), FLOAT_FRACTIONAL_PART(speed, 2), motor);

        return false;
    }

    if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("SetMotorSpeed: Failed to acquire mutex");

        return false;
    }
    
    uint16_t current_speed = g_dynamic_motor_lut[motor].speed;
    eMotorDirection_t current_direction = g_dynamic_motor_lut[motor].direction;
    eMotorControl_t current_control = g_dynamic_motor_lut[motor].mode;

    osMutexRelease(g_dynamic_motor_lut[motor].mutex);

    uint16_t target_speed = Motor_API_ScaleSpeed(motor, speed);
    eMotorDirection_t new_direction = direction;

    if ((target_speed == current_speed) && (current_direction == direction) && (current_control == control)) {
        return true;
    }

    // TODO: There should be a polynomial function to calculate offset
    target_speed *= g_static_motor_lut[motor].motor_speed_offset;

    if (target_speed > g_dynamic_motor_lut[motor].max_speed) {
        target_speed = g_dynamic_motor_lut[motor].max_speed;
    }

    int16_t step_value = 0;

    if ((g_static_motor_lut[motor].rotation[current_direction] != g_static_motor_lut[motor].rotation[direction]) && (eMotorDirection_Stop != current_direction) && (STOP_SPEED != current_speed)) {
        current_speed = STOP_SPEED;
        new_direction = eMotorDirection_Brake;

        if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[new_direction], current_speed)) {
            TRACE_ERR("SetMotorSpeed: Failed to set motor speed for motor [%d]\n", motor);
            
            return false;
        }

        if (osOK != osTimerStart(g_dynamic_motor_lut[motor].timer, MOTOR_BRAKE_TIME_MS)) {
            TRACE_ERR("SetMotorSpeed: Failed to start brake timer for motor [%d]\n", motor);
            osTimerStop(g_dynamic_motor_lut[motor].timer);
            
            return false;
        }
    } else if (eMotorControl_Ramp == control) {
        step_value = (target_speed - current_speed) / MOTOR_RAMP_STEPS;
        
        if (osOK != osTimerStart(g_dynamic_motor_lut[motor].timer, MOTOR_RAMP_TIMER_MS)) {
            TRACE_ERR("SetMotorSpeed: Failed to start ramp timer for motor [%d]\n", motor);
            
            return false;
        }
    } else {
        if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[direction], target_speed)) {
            TRACE_ERR("SetMotorSpeed: Failed to set motor speed for motor [%d]\n", motor);
            
            return false;
        }

        current_speed = target_speed;
    }

    if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("SetMotorSpeed: Failed to acquire mutex");
        osTimerStop(g_dynamic_motor_lut[motor].timer);

        return false;
    }

    g_dynamic_motor_lut[motor].target_direction = direction;
    g_dynamic_motor_lut[motor].direction = new_direction;
    g_dynamic_motor_lut[motor].mode = control;
    g_dynamic_motor_lut[motor].step_value = step_value;
    g_dynamic_motor_lut[motor].target_speed = target_speed;
    g_dynamic_motor_lut[motor].speed = current_speed;

    osMutexRelease(g_dynamic_motor_lut[motor].mutex);

    return true;
}

bool Motor_API_StopAllMotors (void) {
    bool is_success = true;

    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        if (!Motor_API_IsMotorEnabled(motor)) {
            continue;
        }

        if (osTimerIsRunning(g_dynamic_motor_lut[motor].timer)) { 
            osTimerStop(g_dynamic_motor_lut[motor].timer);
        }
        if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
            TRACE_ERR("StopAllMotors: Failed to acquire mutex for motor [%d]\n", motor);
            
            is_success = false;

            continue;
        }

        g_dynamic_motor_lut[motor].direction = eMotorDirection_Stop;
        g_dynamic_motor_lut[motor].speed = STOP_SPEED;

        osMutexRelease(g_dynamic_motor_lut[motor].mutex);

        if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[g_dynamic_motor_lut[motor].direction], STOP_SPEED)) {
            TRACE_ERR("StopAllMotors: Failed to stop motor [%d]\n", motor);
            
            is_success = false;

            continue;
        }
    }

    return is_success;
}

bool Motor_API_EnableAllMotors (void) {
    bool is_success = true;
    
    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        if (Motor_API_IsMotorEnabled(motor)) {
            continue;
        }

        if (!Motor_Driver_EnableMotor(motor)) {
            TRACE_ERR("EnableAllMotors: Failed to enable motor [%d]\n", motor);
            
            is_success = false;
            
            continue;
        }

        if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
            TRACE_ERR("EnableAllMotors: Failed to acquire mutex for motor [%d]\n", motor);
            
            is_success = false;

            continue;
        }

        g_dynamic_motor_lut[motor].is_enabled = true;

        osMutexRelease(g_dynamic_motor_lut[motor].mutex);
    }

    return is_success;
}

bool Motor_API_DisableAllMotors (void) {
    bool is_success = true;
    
    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        if (!Motor_API_IsMotorEnabled(motor)) {
            continue;
        }

        if (!Motor_Driver_DisableMotor(motor)) {
            TRACE_ERR("DisableAllMotors: Failed to disable motor [%d]\n", motor);
            
            is_success = false;

            continue;
        }

        if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
            TRACE_ERR("DisableAllMotors: Failed to acquire mutex for motor [%d]\n", motor);
            
            is_success = false;

            continue;
        }

        g_dynamic_motor_lut[motor].is_enabled = false;

        osMutexRelease(g_dynamic_motor_lut[motor].mutex);
    }

    return is_success;
}

bool Motor_API_IsCorrectSpeed (const float speed) {
    return (speed >= INPUT_MIN_SPEED) && (speed <= INPUT_MAX_SPEED);
}

bool Motor_API_IsCorrectMode (const eMotorControl_t mode) {
    return (mode >= eMotorControl_First) && (mode < eMotorControl_Last);
}

bool Motor_API_IsMotorEnabled (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        TRACE_ERR("IsMotorEnabled: Incorrect motor type [%d]\n", motor);
        
        return false;
    }

    bool is_enabled = false;

    if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("IsMotorEnabled: Failed to acquire mutex for motor [%d]\n", motor);
        
        return false;
    }

    is_enabled = g_dynamic_motor_lut[motor].is_enabled;

    osMutexRelease(g_dynamic_motor_lut[motor].mutex);

    return is_enabled;
}

bool Motor_API_GetMotorRotation (const eMotor_t motor, eMotorRotation_t *rotation) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        TRACE_ERR("GetMotorRotation: Incorrect motor type [%d]\n", motor);
        
        return false;
    }

    if (rotation == NULL) {
        TRACE_ERR("GetMotorRotation: NULL argument\n");
        
        return false;
    }

    if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("GetMotorRotation: Failed to acquire mutex for motor [%d]\n", motor);
        
        return false;
    }

    *rotation = g_static_motor_lut[motor].rotation[g_dynamic_motor_lut[motor].direction];

    osMutexRelease(g_dynamic_motor_lut[motor].mutex);

    return true;
}

#endif /* ENABLE_MOTOR */
