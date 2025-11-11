/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "animation_segmentfill.h"

#ifdef ENABLE_LED_ANIMATION

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

void Animation_SegmentFill_FillBuffer (sSegmentFillData_t *data);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
void Animation_SegmentFill_FillBuffer (sSegmentFillData_t *data) {
    if (data == NULL) {
        return;
    }
    
    if (!WS2812B_Config_IsCorrectWs2812b(data->device)) {
        return;
    }

    if (data->brightness == 0) {
        return;
    }

    uint8_t r_base = (data->base_rgb.color >> 16) & 0xFF;
    uint8_t g_base = (data->base_rgb.color >> 8) & 0xFF;
    uint8_t b_base = data->base_rgb.color & 0xFF;

    uint8_t r_segment = (data->segment_rgb.color >> 16) & 0xFF;
    uint8_t g_segment = (data->segment_rgb.color >> 8) & 0xFF;
    uint8_t b_segment = data->segment_rgb.color & 0xFF;

    if (r_base != 0 || g_base != 0 || b_base != 0) {
        r_base = LED_ScaleBrightness(r_base, data->brightness);
        g_base = LED_ScaleBrightness(g_base, data->brightness);
        b_base = LED_ScaleBrightness(b_base, data->brightness);

        WS2812B_API_FillColor(data->device, r_base, g_base, b_base);
    }

    if (r_segment != 0 || g_segment != 0 || b_segment != 0) {
        r_segment = LED_ScaleBrightness(r_segment, data->brightness);
        g_segment = LED_ScaleBrightness(g_segment, data->brightness);
        b_segment = LED_ScaleBrightness(b_segment, data->brightness);
    }
    
    if ((data->end_led - data->start_led) == 1) {
        WS2812B_API_SetColor(data->device, data->start_led, r_segment, g_segment, b_segment);
        
        return;
    }

    WS2812B_API_FillSegment(data->device, data->start_led, data->end_led, r_segment, g_segment, b_segment);

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void Animation_SegmentFill_Run (void *context) {
    if (context == NULL) {
        return;
    }

    Animation_SegmentFill_FillBuffer((sSegmentFillData_t *)context);

    return;
}

#endif /* ENABLE_LED_ANIMATION */
