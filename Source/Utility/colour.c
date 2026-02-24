/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "colour.h"

#if defined(ENABLE_COLOUR)
#include <stddef.h>

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
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void Colour_HsvToRgb (const sColourHsv_t hsv, ColourRgb_t *rgb) {
    if (NULL == rgb) {
        return;
    }

    uint8_t red = 0, green = 0, blue = 0;

    if (0 == hsv.saturation) {
        red = hsv.value;
        green = hsv.value;
        blue = hsv.value;
    } else {
        uint8_t region = hsv.hue / 43;
        uint8_t remainder = (hsv.hue - region * 43) * 6;

        uint8_t p = (hsv.value * (255 - hsv.saturation)) >> 8;
        uint8_t q = (hsv.value * (255 - ((hsv.saturation * remainder) >> 8))) >> 8;
        uint8_t t = (hsv.value * (255 - ((hsv.saturation * (255 - remainder)) >> 8))) >> 8;

        switch (region) {
            case 0: {
                red = hsv.value;
                green = t;
                blue = p;
            } break;
            case 1: {
                red = q;
                green = hsv.value;
                blue = p;
            } break;
            case 2: {
                red = p;
                green = hsv.value;
                blue = t;
            } break;
            case 3: {
                red = p;
                green = q;
                blue = hsv.value;
            } break;
            case 4: {
                red = t;
                green = p;
                blue = hsv.value;
            } break;
            default: {
                red = hsv.value;
                green = p;
                blue = q;
            } break;
        }
    }

    *rgb = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;

    return;
}

void Colour_RgbToHsv(const ColourRgb_t rgb, sColourHsv_t *hsv) {
    if (NULL == hsv) {
        return;
    }

    uint8_t red = (rgb >> 16) & 0xFF;
    uint8_t green = (rgb >> 8) & 0xFF;
    uint8_t blue = rgb & 0xFF;

    uint8_t rgb_min = red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);
    uint8_t rgb_max = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
    
    uint8_t delta = rgb_max - rgb_min;

    hsv->value = rgb_max;

    if (0 == rgb_max) {
        hsv->saturation = 0;
        hsv->hue = 0;
        return;
    }

    hsv->saturation = (delta * 255) / rgb_max;

    if (0 == delta) {
        hsv->hue = 0;
        return;
    }

    int16_t hue;

    if (red == rgb_max) {
        hue = 0 + 43 * (green - blue) / delta;
    } else if (green == rgb_max) {
        hue = 85 + 43 * (blue - red) / delta;
    } else {
        hue = 171 + 43 * (red - green) / delta;
    }

    if (hue < 0) hue += 256;

    hsv->hue = (uint8_t)hue;

    return;
}

uint8_t Colour_ScaleBrightness (const uint8_t value, const uint8_t brightness) {
    if (0 == brightness) {
        return 0;
    }

    if (brightness > MAX_BRIGHTNESS) {
        return value;
    }

    return (value * brightness) / MAX_BRIGHTNESS;
}

#endif /* ENABLE_COLOUR */
