#ifndef SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SOLIDcolour_H_
#define SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SOLIDcolour_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_LED_ANIMATION)
#include <stdint.h>
#include "ws2812b_api.h"
#include "colour.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef struct sSolidAnimationData {
    eWs2812b_t device;
    ColourRgb_t rgb;
    uint8_t brightness;
} sSolidAnimationData_t;

typedef struct sLedAnimationSolidColour {
    ColourRgb_t rgb;
} sLedAnimationSolidColour_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Animation_SolidColour_Run (void *context);

#endif /* ENABLE_LED_ANIMATION */
#endif /* SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SOLIDcolour_H_ */
