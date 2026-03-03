/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "animation_rainbow.h"

#ifdef ENABLE_LED_ANIMATION
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

void Animation_Rainbow_FillBuffer (sLedRainbow_t *context);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

void Animation_Rainbow_FillBuffer (sLedRainbow_t *context) {
    if (context == NULL) {
        return;
    }

    if (context->parameters == NULL) {
        return;
    }
    
    if (!WS2812B_Config_IsCorrectWs2812b(context->device)) {
        return;
    }

    if (context->brightness == 0) {
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

            context->hue_offset = rainbow_data->start_hsv_color.hue;

            context->frame_counter = 0;

            context->state = eRainbowState_Run;
        }
        case eRainbowState_Run: {
            if (context->frame_counter < rainbow_data->frames_per_update) {
                context->frame_counter++;
                
                return;
            }
            
            sLedColorHsv_t hsv = {
                .saturation = rainbow_data->start_hsv_color.saturation,
                .value = rainbow_data->start_hsv_color.value
            };

            sLedColorRgb_t rgb = {0};

            uint8_t red;
            uint8_t green;
            uint8_t blue;

            if (rainbow_data->hue_step == 0) {
                hsv.hue = context->hue_offset;

                LED_HsvToRgb(hsv, &rgb);

                red = LED_ScaleBrightness(((rgb.color >> 16) & 0xFF), context->brightness);
                green = LED_ScaleBrightness(((rgb.color >> 8) & 0xFF), context->brightness);
                blue = LED_ScaleBrightness((rgb.color & 0xFF), context->brightness);
                
                for (size_t led = rainbow_data->segment_start_led; led <= rainbow_data->segment_end_led; led++) {
                    if (!WS2812B_API_SetColor(context->device, led, red, green, blue)) {
                        context->state = eRainbowState_Init;
                        
                        return;
                    }
                }
            } else {
                for (size_t led = rainbow_data->segment_start_led; led <= rainbow_data->segment_end_led; led++) {
                    hsv.hue = context->hue_offset + led * rainbow_data->hue_step;

                    LED_HsvToRgb(hsv, &rgb);

                    red = LED_ScaleBrightness(((rgb.color >> 16) & 0xFF), context->brightness);
                    green = LED_ScaleBrightness(((rgb.color >> 8) & 0xFF), context->brightness);
                    blue = LED_ScaleBrightness((rgb.color & 0xFF), context->brightness);

                    if (!WS2812B_API_SetColor(context->device, led, red, green, blue)) {
                        context->state = eRainbowState_Init;

                        return;
                    }
                }
            }

            if (rainbow_data->direction == eDirection_Up) {
                context->hue_offset -= rainbow_data->speed;
            } else {
                context->hue_offset += rainbow_data->speed;
            }

            context->frame_counter = 0;
        } break;
        default: {
            return;
        }
    }
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void Animation_Rainbow_Run (void *context) {
    if (context == NULL) {
        return;
    }

    Animation_Rainbow_FillBuffer((sLedRainbow_t*) context);

    return;
}

void Animation_Rainbow_Free (void *context) {
    if (context == NULL) {
        return;
    }

    sLedRainbow_t *rainbow = (sLedRainbow_t*) context;

    if (rainbow->parameters != NULL) {
        WS2812B_API_FreeData(rainbow->parameters);
    }

    WS2812B_API_FreeData(rainbow);

    return;
}

bool Animation_Rainbow_IsCorrectSpeed (const uint8_t speed) {
    return (speed != 0);
}

#endif /* ENABLE_LED_ANIMATION */
