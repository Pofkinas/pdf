/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "animation_segmentfill.h"

#if defined(ENABLE_LED_ANIMATION)

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

static void Animation_SegmentFill_FillBuffer (sSegmentFillData_t *data);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void Animation_SegmentFill_FillBuffer (sSegmentFillData_t *data) {
    if (NULL == data) {
        return;
    }
    
    if (!WS2812B_Config_IsCorrectWs2812b(data->device)) {
        return;
    }

    if (0 == data->brightness) {
        return;
    }

    uint8_t r_base = (data->base_rgb >> RGB_RED_SHIFT) & RGB_BYTE_MASK;
    uint8_t g_base = (data->base_rgb >> RGB_GREEN_SHIFT) & RGB_BYTE_MASK;
    uint8_t b_base = data->base_rgb & RGB_BYTE_MASK;

    uint8_t r_segment = (data->segment_rgb >> RGB_RED_SHIFT) & RGB_BYTE_MASK;
    uint8_t g_segment = (data->segment_rgb >> RGB_GREEN_SHIFT) & RGB_BYTE_MASK;
    uint8_t b_segment = data->segment_rgb & RGB_BYTE_MASK;

    if ((0 != r_base) || (0 != g_base) || (0 != b_base)) {
        r_base = Colour_ScaleBrightness(r_base, data->brightness);
        g_base = Colour_ScaleBrightness(g_base, data->brightness);
        b_base = Colour_ScaleBrightness(b_base, data->brightness);

        WS2812B_API_FillColour(data->device, r_base, g_base, b_base);
    }

    if ((0 != r_segment) || (0 != g_segment) || (0 != b_segment)) {
        r_segment = Colour_ScaleBrightness(r_segment, data->brightness);
        g_segment = Colour_ScaleBrightness(g_segment, data->brightness);
        b_segment = Colour_ScaleBrightness(b_segment, data->brightness);
    }
    
    if (1 == (data->end_led - data->start_led)) {
        WS2812B_API_SetColour(data->device, data->start_led, r_segment, g_segment, b_segment);
        
        return;
    }

    WS2812B_API_FillSegment(data->device, data->start_led, data->end_led, r_segment, g_segment, b_segment);

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void Animation_SegmentFill_Run (void *context) {
    if (NULL == context) {
        return;
    }

    Animation_SegmentFill_FillBuffer((sSegmentFillData_t*)context);

    return;
}

#endif /* ENABLE_LED_ANIMATION */
