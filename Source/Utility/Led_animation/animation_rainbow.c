/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "animation_rainbow.h"

#if defined(ENABLE_LED_ANIMATION)
#include <stdlib.h>

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

static void Animation_Rainbow_FillBuffer (sLedRainbow_t *context);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void Animation_Rainbow_FillBuffer (sLedRainbow_t *context) {
    if (NULL == context) {
        return;
    }

    if (NULL == context->parameters) {
        return;
    }
    
    if (!WS2812B_Config_IsCorrectWs2812b(context->device) || (0 == context->brightness)) {
        return;
    }

    sLedAnimationRainbow_t *rainbow_data = context->parameters;

    switch (context->state) {
        case eRainbowState_Init: {
            if (rainbow_data->segment_start_led >= rainbow_data->segment_end_led) {
                return;
            }
        
            if (!Animation_Rainbow_IsCorrectSpeed(rainbow_data->speed)) {
                return;
            }

            context->hue_offset = rainbow_data->start_hsv_colour.hue;
            context->frame_counter = 0;
            context->state = eRainbowState_Run;
            /* fall through */
        }
        case eRainbowState_Run: {
            if (0 != (context->frame_counter % rainbow_data->frames_per_update)) {
                context->frame_counter++;
                
                return;
            }
            
            sColourHsv_t hsv = {
                .saturation = rainbow_data->start_hsv_colour.saturation,
                .value = rainbow_data->start_hsv_colour.value
            };

            ColourRgb_t rgb = {0};

            uint8_t red;
            uint8_t green;
            uint8_t blue;

            if (0 == rainbow_data->hue_step) {
                hsv.hue = context->hue_offset;

                Colour_HsvToRgb(hsv, &rgb);

                red = Colour_ScaleBrightness(((rgb >> RGB_RED_SHIFT) & RGB_BYTE_MASK), context->brightness);
                green = Colour_ScaleBrightness(((rgb >> RGB_GREEN_SHIFT) & RGB_BYTE_MASK), context->brightness);
                blue = Colour_ScaleBrightness((rgb & RGB_BYTE_MASK), context->brightness);
                
                for (size_t led = rainbow_data->segment_start_led; led <= rainbow_data->segment_end_led; led++) {
                    if (!WS2812B_API_SetColour(context->device, led, red, green, blue)) {
                        context->state = eRainbowState_Init;
                        
                        return;
                    }
                }
            } else {
                for (size_t led = rainbow_data->segment_start_led; led <= rainbow_data->segment_end_led; led++) {
                    hsv.hue = context->hue_offset + led * rainbow_data->hue_step;

                    Colour_HsvToRgb(hsv, &rgb);

                    red = Colour_ScaleBrightness(((rgb >> RGB_RED_SHIFT) & RGB_BYTE_MASK), context->brightness);
                    green = Colour_ScaleBrightness(((rgb >> RGB_GREEN_SHIFT) & RGB_BYTE_MASK), context->brightness);
                    blue = Colour_ScaleBrightness((rgb & RGB_BYTE_MASK), context->brightness);

                    if (!WS2812B_API_SetColour(context->device, led, red, green, blue)) {
                        context->state = eRainbowState_Init;

                        return;
                    }
                }
            }

            if (eDirection_Up == rainbow_data->direction) {
                context->hue_offset -= rainbow_data->speed;
            } else {
                context->hue_offset += rainbow_data->speed;
            }
        } break;
        default: {}
    }

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void Animation_Rainbow_Run (void *context) {
    if (NULL == context) {
        return;
    }

    Animation_Rainbow_FillBuffer((sLedRainbow_t*) context);

    return;
}

void Animation_Rainbow_Free (void *context) {
    if (NULL == context) {
        return;
    }

    sLedRainbow_t *rainbow = (sLedRainbow_t*) context;

    if (NULL != rainbow->parameters) {
        WS2812B_API_FreeData(rainbow->parameters);
    }

    WS2812B_API_FreeData(rainbow);

    return;
}

bool Animation_Rainbow_IsCorrectSpeed (const uint8_t speed) {
    return (0 != speed);
}

#endif /* ENABLE_LED_ANIMATION */
