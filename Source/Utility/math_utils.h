#ifndef SOURCE_UTILITY_MATH_UTILS_H_
#define SOURCE_UTILITY_MATH_UTILS_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdint.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef struct sPID {
    float kp, ki, kd;           // Gains
    float integral;              // Accumulated error
    float prev_error;            // Last error for derivative
    float integral_limit;        // Anti-windup limit
    float output_min, output_max; // Output saturation
} sPID_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

uint32_t Math_Utils_RandomRange (uint32_t min, uint32_t max);
uint32_t Math_Utils_MapValue (uint32_t input, uint32_t input_min, uint32_t input_max, uint32_t output_min, uint32_t output_max);
float Math_Utils_DegreesToRadians (float degrees);
float Math_Utils_RadiansToDegrees (float radians);
float Math_Utils_PID_Update (sPID_t *pid, float set_point, float process_value, float dt);

#endif /* SOURCE_UTILITY_MATH_UTILS_H_ */
