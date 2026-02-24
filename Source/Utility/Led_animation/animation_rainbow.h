#ifndef SOURCE_UTILITY_LED_ANIMATION_ANIMATION_RAINBOW_H_
#define SOURCE_UTILITY_LED_ANIMATION_ANIMATION_RAINBOW_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_LED_ANIMATION)
#include <stdint.h>
#include <stddef.h>
#include "ws2812b_api.h"
#include "colour.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eRainbowState {
    eRainbowState_First = 0,
    eRainbowState_Init = eRainbowState_First,
    eRainbowState_Run,
    eRainbowState_Last
} eRainbowState_t;

typedef struct sLedRainbow {
    eWs2812b_t device;
    uint8_t brightness;
    eRainbowState_t state;
    sLedAnimationRainbow_t *parameters;
    
    size_t frame_counter;
    uint8_t hue_offset;
} sLedRainbow_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Animation_Rainbow_Run (void *context);
void Animation_Rainbow_Free (void *context);
bool Animation_Rainbow_IsCorrectSpeed (const uint8_t speed);

#endif /* ENABLE_LED_ANIMATION */
#endif /* SOURCE_UTILITY_LED_ANIMATION_ANIMATION_RAINBOW_H_ */
