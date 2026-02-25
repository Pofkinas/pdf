#ifndef SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SEGMENTFILL_H_
#define SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SEGMENTFILL_H_
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

typedef struct sSegmentFillData {
    eWs2812b_t device;
    uint8_t brightness;
    ColourRgb_t base_rgb;
    ColourRgb_t segment_rgb;
    size_t start_led;
    size_t end_led;
} sSegmentFillData_t;

typedef struct sLedAnimationSegmentFill {
    ColourRgb_t rgb_base;
    ColourRgb_t rgb_segment;
    size_t segment_start_led;
    size_t segment_end_led;
} sLedAnimationSegmentFill_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Animation_SegmentFill_Run (void *context);

#endif /* ENABLE_LED_ANIMATION */
#endif /* SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SEGMENTFILL_H_ */
