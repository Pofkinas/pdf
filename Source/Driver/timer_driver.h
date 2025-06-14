#ifndef SOURCE_DRIVER_TIMER_DRIVER_H_
#define SOURCE_DRIVER_TIMER_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "framework_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eTimerDriver {
    eTimerDriver_First = 0,
    eTimerDriver_TIM10,

    #if defined(USE_MOTOR_A) || defined(USE_MOTOR_B) || defined(USE_PULSE_LED)
    eTimerDriver_TIM3,
    #endif

    #if defined(USE_WS2812B_1) || defined(USE_WS2812B_2)
    eTimerDriver_TIM5,
    #endif

    eTimerDriver_Last
} eTimerDriver_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Timer_Driver_DefineTimers (const sTimerDesc_t *timer_lut);
bool Timer_Driver_InitAllTimers (void);
bool Timer_Driver_Start (const eTimerDriver_t timer);
bool Timer_Driver_Stop (const eTimerDriver_t timer);
uint16_t Timer_Driver_GetResolution (const eTimerDriver_t timer);

#endif /* SOURCE_DRIVER_TIMER_DRIVER_H_ */
