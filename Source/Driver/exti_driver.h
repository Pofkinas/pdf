#ifndef SOURCE_DRIVER_EXTI_DRIVER_H_
#define SOURCE_DRIVER_EXTI_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include "framework_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eExtiDriver {
    eExtiDriver_First = 0,

    #ifdef USE_START_BUTTON
    eExtiDriver_StartButton,
    #endif

    #ifdef USE_TCRT5000_RIGHT
    eExtiDriver_Tcrt5000_Right,
    #endif

    #ifdef USE_TCRT5000_LEFT
    eExtiDriver_Tcrt5000_Left,
    #endif

    eExtiDriver_Last
} eExtiDriver_t;
/* clang-format on */

typedef void (*exti_callback_t) (void *context);

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Exti_Driver_DefinePerips (const sExtiStaticDesc_t *exti_lut);
bool Exti_Driver_InitDevice (eExtiDriver_t exti_device, exti_callback_t exti_callback, void *callback_context);
bool Exti_Driver_Disable_IT (const eExtiDriver_t exti_device);
bool Exti_Driver_Enable_IT (const eExtiDriver_t exti_device);
bool Exti_Driver_ClearFlag (const eExtiDriver_t exti_device);

#endif /* SOURCE_DRIVER_EXTI_DRIVER_H_ */
