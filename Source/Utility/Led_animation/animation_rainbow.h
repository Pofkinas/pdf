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

typedef enum eRainbowState {
    eRainbowState_First = 0,
    eRainbowState_Init = eRainbowState_First,
    eRainbowState_Run,
    eRainbowState_Last
} eRainbowState_t;

typedef struct sLedAnimationRainbow {
    eDirection_t direction;
    sColourHsv_t start_hsv_colour;
    size_t segment_start_led;
    size_t segment_end_led;
    uint8_t speed;
    uint8_t hue_step;
    size_t frames_per_update;
} sLedAnimationRainbow_t;

typedef struct sLedRainbow {
    eWs2812b_t device;
    uint8_t brightness;
    uint8_t hue_offset;
    eRainbowState_t state;
    sLedAnimationRainbow_t *parameters;
    uint32_t frame_counter;
} sLedRainbow_t;

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
