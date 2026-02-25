/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "math_utils.h"

#include <math.h>
#include <stdlib.h>

#include "framework_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
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

uint32_t Math_Utils_RandomRange (uint32_t min, uint32_t max) {
    if (min > max) {
        return 0;
    }

    return (rand() % (max + 1 - min)) + min;
}

uint32_t Math_Utils_MapValue (uint32_t input, uint32_t input_min, uint32_t input_max, uint32_t output_min, uint32_t output_max) {
    if ((input < input_min) || (input > input_max)) {
        return 0;
    }

    return ((((input - input_min) * (output_max - output_min)) / (input_max - input_min)) + output_min);
}

float Math_Utils_DegreesToRadians (float degrees) {
    return (degrees * (M_PI / 180.0f));
}

float Math_Utils_RadiansToDegrees (float radians) {
    return (radians * (180.0f / M_PI));
}

float Math_Utils_PidUpdate (sPID_t *pid, float set_point, float process_value, float dt) {
    if ((NULL == pid) || (dt <= 0.0f)) {
        return 0.0f;
    }
    
    if (dt > MAX_PID_DT) {
        dt = MAX_PID_DT;
    }

    float error = set_point - process_value;

    // Proportional
    float p_term = pid->kp * error;

    // Integral with anti-windup
    pid->integral += error * dt;
    
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }

    float i_term = pid->ki * pid->integral;

    // Derivative
    float derivative = (error - pid->prev_error) / dt;
    float d_term = pid->kd * derivative;

    pid->prev_error = error;
    float output = p_term + i_term + d_term;

    // Apply output saturation
    if (output > pid->output_max) {
        output = pid->output_max;
    } else if (output < pid->output_min) {
        output = pid->output_min;
    }

    return output;
}
