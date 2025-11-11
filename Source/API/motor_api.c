/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_api.h"

#ifdef ENABLE_MOTOR
#include <stdint.h>
#include "cmsis_os2.h"
#include "debug_api.h"
#include "motor_driver.h"
#include "pwm_driver.h"
#include "timer_driver.h"
#include "gpio_driver.h"

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
    uint16_t new_speed;
    size_t max_speed;
    eMotorDirection_t direction;
    osMutexId_t mutex;
    bool is_soft_start_running;
    osTimerId_t soft_start_timer;
    size_t step_value;
    uint8_t soft_start_step;
} sMotorDynamic_t;

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static uint16_t Motor_API_Scale_Speed (const eMotor_t motor, const size_t speed);
static bool Motor_API_SetMotorSpeed (const eMotor_t motor, const size_t speed, const eMotorDirection_t direction);
static void Motor_API_Statup_TimerCallback (void *arg);

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
 * Definitions of private functions
 *********************************************************************************************************************/
 
static uint16_t Motor_API_Scale_Speed (const eMotor_t motor, const size_t speed) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        TRACE_ERR("Motor_API_Scale_Speed: Incorrect motor type %d\n", motor);
        
        return 0;
    }

    if (!Motor_API_IsCorrectSpeed(speed)) {
        TRACE_ERR("Motor_API_Scale_Speed: Incorrect speed %d\n", speed);
        
        return 0;
    }
    
    if (speed == INPUT_MIN_SPEED) {
        return 0;
    }

    if (speed == INPUT_MAX_SPEED) {
        return (uint16_t) ((uint32_t) MAX_SCALED_SPEED * g_dynamic_motor_lut[motor].max_speed / INPUT_MAX_SPEED);
    }

    uint16_t scaled_speed = MIN_SCALED_SPEED + ((speed * (MAX_SCALED_SPEED - MIN_SCALED_SPEED)) / MAX_SCALED_SPEED);

    return (uint16_t) ((uint32_t) scaled_speed * g_dynamic_motor_lut[motor].max_speed / INPUT_MAX_SPEED);
}

static bool Motor_API_SetMotorSpeed (const eMotor_t motor, const size_t speed, const eMotorDirection_t direction) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        TRACE_ERR("Motor_API_SetMotorSpeed: Incorrect motor type %d\n", motor);
        
        return false;
    }

    if ((speed < STOP_SPEED) || (speed > g_dynamic_motor_lut[motor].max_speed)) {
        TRACE_ERR("Motor_API_SetMotorSpeed: Speed %d out of range for motor %d\n", speed, motor);
        
        return false;
    }

    if (!Motor_Config_IsCorrectDirection(direction)) {
        TRACE_ERR("Motor_API_SetMotorSpeed: Incorrect direction %d\n", direction);
        
        return false;
    }

    if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Motor_API_SetMotorSpeed: Failed to acquire mutex for motor %d\n", motor);
        
        return false;
    }

    switch (direction) {
        case eMotorDirection_Forward: {
            if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[eMotorDirection_Forward], speed)) {
                TRACE_ERR("Motor_API_SetMotorSpeed: Failed to set forward speed for motor %d\n", motor);
                
                return false;
            }
        } break;
        case eMotorDirection_Reverse: {
            if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[eMotorDirection_Reverse], speed)) {
                TRACE_ERR("Motor_API_SetMotorSpeed: Failed to set reverse speed for motor %d\n", motor);
                
                return false;
            }
        } break;
        case eMotorDirection_Right: {
            if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[eMotorDirection_Right], speed)) {
                TRACE_ERR("Motor_API_SetMotorSpeed: Failed to set right speed for motor %d\n", motor);
                
                return false;
            }
        } break;
        case eMotorDirection_Left: {
            if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[eMotorDirection_Left], speed)) {
                TRACE_ERR("Motor_API_SetMotorSpeed: Failed to set left speed for motor %d\n", motor);
                
                return false;
            }
        } break;
        default: {
        } break;
    }

    osMutexRelease(g_dynamic_motor_lut[motor].mutex);

    return true;
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
            TRACE_ERR("Init: No motor desc for motor %d\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }

        g_static_motor_lut[motor] = *desc;
        
        g_dynamic_motor_lut[motor].mutex = osMutexNew(&g_static_motor_lut[motor].mutex_attributes);

        if (g_dynamic_motor_lut[motor].mutex == NULL) {
            TRACE_ERR("Init: Failed to create mutex for motor %d\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }

        g_dynamic_motor_lut[motor].soft_start_timer = osTimerNew(Motor_API_Statup_TimerCallback, osTimerPeriodic, &g_dynamic_motor_lut[motor], &g_static_motor_lut[motor].timer_attributes);

        if (g_dynamic_motor_lut[motor].soft_start_timer == NULL) {
            TRACE_ERR("Init: Failed to create soft start timer for motor %d\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }

        if (!Motor_Driver_GetMaxSpeed(motor, &g_dynamic_motor_lut[motor].max_speed)) {
            TRACE_ERR("Init: Failed to get max speed for motor %d\n", motor);
            
            g_is_all_motors_init = false;

            continue;
        }

        g_dynamic_motor_lut[motor].motor = motor;
        g_dynamic_motor_lut[motor].direction = eMotorDirection_Last;
        g_dynamic_motor_lut[motor].is_soft_start_running = false;
        g_dynamic_motor_lut[motor].soft_start_step = DEFAULT_SOFT_START_STEP;
    }

    return g_is_all_motors_init;
}

bool Motor_API_SetSpeed (const size_t speed, const eMotorDirection_t direction) {
    if (!Motor_API_IsCorrectSpeed(speed)) {
        TRACE_ERR("SetSpeed: Incorrect speed %d\n", speed);
        
        return false;
    }

    if (!Motor_Config_IsCorrectDirection(direction)) {
        TRACE_ERR("SetSpeed: Incorrect direction %d\n", direction);
        
        return false;
    }

    bool is_success = true;

    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        if (!Motor_API_IsMotorEnabled(motor)) {
            if (!Motor_Driver_EnableMotor(motor)) {
                TRACE_ERR("SetSpeed: Failed to enable motor %d\n", motor);
                
                is_success = false;

                continue;
            }

            g_dynamic_motor_lut[motor].is_enabled = true;
        }

        if (g_dynamic_motor_lut[motor].is_soft_start_running) {
            continue;
        }

        if (osMutexAcquire(g_dynamic_motor_lut[motor].mutex, MUTEX_TIMEOUT) != osOK) {
            TRACE_ERR("SetSpeed: Failed to acquire mutex for motor %d\n", motor);
            
            is_success = false;

            continue;
        }

        g_dynamic_motor_lut[motor].new_speed = Motor_API_Scale_Speed(motor, speed);

        if (direction == eMotorDirection_RightSoft && g_static_motor_lut[motor].position == eMotorPosition_Left) {
            g_dynamic_motor_lut[motor].new_speed -= SOFT_TURN_SPEED_OFFSET;
        } else if (direction == eMotorDirection_LeftSoft && g_static_motor_lut[motor].position == eMotorPosition_Right) {
            g_dynamic_motor_lut[motor].new_speed -= SOFT_TURN_SPEED_OFFSET;
        }

        if (g_dynamic_motor_lut[motor].new_speed == g_dynamic_motor_lut[motor].speed && g_dynamic_motor_lut[motor].direction == direction) {
            osMutexRelease(g_dynamic_motor_lut[motor].mutex);
            
            continue;
        }

        g_dynamic_motor_lut[motor].new_speed += g_static_motor_lut[motor].motor_speed_offset;

        if (g_dynamic_motor_lut[motor].new_speed > g_dynamic_motor_lut[motor].max_speed) {
            g_dynamic_motor_lut[motor].new_speed = g_dynamic_motor_lut[motor].max_speed;
        }

        g_dynamic_motor_lut[motor].direction = direction;

        if (g_dynamic_motor_lut[motor].speed == STOP_SPEED) {
            g_dynamic_motor_lut[motor].soft_start_step = DEFAULT_SOFT_START_STEP;
            g_dynamic_motor_lut[motor].step_value = g_dynamic_motor_lut[motor].new_speed / MOTOR_SOFT_START_STEPS;
            g_dynamic_motor_lut[motor].is_soft_start_running = true;
        }

        osMutexRelease(g_dynamic_motor_lut[motor].mutex);

        if (g_dynamic_motor_lut[motor].speed == STOP_SPEED) {
            osTimerStart(g_dynamic_motor_lut[motor].soft_start_timer, MOTOR_SOFT_START_TIMER_MS);
        } else {
            if (!Motor_API_SetMotorSpeed(motor, g_dynamic_motor_lut[motor].new_speed, g_dynamic_motor_lut[motor].direction)) {
                TRACE_ERR("SetSpeed: Failed to set motor speed for motor %d\n", motor);
                
                is_success = false;

                continue;
            }
        }

        g_dynamic_motor_lut[motor].speed = g_dynamic_motor_lut[motor].new_speed;
    }

    return is_success;
}

bool Motor_API_StopAllMotors (void) {
    bool is_success = true;
    
    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        if (!Motor_API_IsMotorEnabled(motor)) {
            TRACE_WRN("StopAllMotors: Motor %d not enabled\n", motor);
            
            is_success = false;

            continue;
        }

        g_dynamic_motor_lut[motor].speed = STOP_SPEED;

        if (!Motor_Driver_SetSpeed(motor, g_static_motor_lut[motor].rotation[eMotorDirection_Forward], g_dynamic_motor_lut[motor].speed)) {
            TRACE_ERR("StopAllMotors: Failed to stop motor %d\n", motor);
            
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
            TRACE_ERR("EnableAllMotors: Failed to enable motor %d\n", motor);
            
            is_success = false;
            
            continue;
        }

        g_dynamic_motor_lut[motor].is_enabled = true;
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
            TRACE_ERR("DisableAllMotors: Failed to disable motor %d\n", motor);
            
            is_success = false;

            continue;
        }

        g_dynamic_motor_lut[motor].is_enabled = false;
    }

    return is_success;
}

bool Motor_API_IsCorrectSpeed (const size_t speed) {
    return (speed >= INPUT_MIN_SPEED) && (speed <= INPUT_MAX_SPEED);
}

bool Motor_API_IsMotorEnabled (const eMotor_t motor) {
    return g_dynamic_motor_lut[motor].is_enabled;
}

#endif /* ENABLE_MOTOR */
