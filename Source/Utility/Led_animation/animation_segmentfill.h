#ifndef SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SEGMENTFILL_H_
#define SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SEGMENTFILL_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_LED_ANIMATION
#include <stdint.h>
#include <stddef.h>
#include "ws2812b_config.h"
#include "ws2812b_api.h"
#include "led_color.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef struct sSegmentFillData {
    eWs2812b_t device;
    uint8_t brightness;
    sLedColorRgb_t base_rgb;
    sLedColorRgb_t segment_rgb;
    size_t start_led;
    size_t end_led;
} sSegmentFillData_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Animation_SegmentFill_Run (void *context);

#endif /* ENABLE_LED_ANIMATION */
#endif /* SOURCE_UTILITY_LED_ANIMATION_ANIMATION_SEGMENTFILL_H_ */
