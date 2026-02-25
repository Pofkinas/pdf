/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "colour.h"

#if defined(ENABLE_COLOUR)
#include <stddef.h>

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/// HSV uses a 0-255 hue range instead of 0-360 degrees.
/// The colour wheel has 6 sectors; 256 / 6 = 42.67, rounded to 43.
#define HSV_HUE_SECTOR_SIZE 43U

/// Number of colour sectors in the HSV wheel.
#define HSV_SECTOR_COUNT 6U

/// 8-bit maximum value used for saturation/value normalisation.
#define HSV_CHANNEL_MAX 255U

/// Shift used to normalise the product of two 8-bit values back to [0, 255].
#define HSV_NORMALISE_SHIFT 8U

/// Hue offset for green sector: 120 / 360 * 256 = 85.3 -> 85
#define HSV_HUE_OFFSET_GREEN 85U

/// Hue offset for blue sector: 240 / 360 * 256 = 170.7 -> 171
#define HSV_HUE_OFFSET_BLUE 171U

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
        uint8_t region = hsv.hue / HSV_HUE_SECTOR_SIZE;
        uint8_t remainder = (hsv.hue - region * HSV_HUE_SECTOR_SIZE) * HSV_SECTOR_COUNT;

        uint8_t pure = (hsv.value * (HSV_CHANNEL_MAX - hsv.saturation)) >> HSV_NORMALISE_SHIFT;
        uint8_t quasi = (hsv.value * (HSV_CHANNEL_MAX - ((hsv.saturation * remainder) >> HSV_NORMALISE_SHIFT))) >> HSV_NORMALISE_SHIFT;
        uint8_t tint = (hsv.value * (HSV_CHANNEL_MAX - ((hsv.saturation * (HSV_CHANNEL_MAX - remainder)) >> HSV_NORMALISE_SHIFT))) >> HSV_NORMALISE_SHIFT;

        switch (region) {
            case 0: {
                red = hsv.value;
                green = tint;
                blue = pure;
            } break;
            case 1: {
                red = quasi;
                green = hsv.value;
                blue = pure;
            } break;
            case 2: {
                red = pure;
                green = hsv.value;
                blue = tint;
            } break;
            case 3: {
                red = pure;
                green = quasi;
                blue = hsv.value;
            } break;
            case 4: {
                red = tint;
                green = pure;
                blue = hsv.value;
            } break;
            default: {
                red = hsv.value;
                green = pure;
                blue = quasi;
            } break;
        }
    }

    *rgb = ((uint32_t)red << RGB_RED_SHIFT) | ((uint32_t)green << RGB_GREEN_SHIFT) | blue;

    return;
}

void Colour_RgbToHsv (const ColourRgb_t rgb, sColourHsv_t *hsv) {
    if (NULL == hsv) {
        return;
    }

    uint8_t red = (rgb >> RGB_RED_SHIFT) & RGB_BYTE_MASK;
    uint8_t green = (rgb >> RGB_GREEN_SHIFT) & RGB_BYTE_MASK;
    uint8_t blue = (rgb >> RGB_BLUE_SHIFT) & RGB_BYTE_MASK;

    uint8_t rgb_min = red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);
    uint8_t rgb_max = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
    
    uint8_t delta = rgb_max - rgb_min;

    hsv->value = rgb_max;

    if (0 == rgb_max) {
        hsv->saturation = 0;
        hsv->hue = 0;
        return;
    }

    hsv->saturation = (delta * HSV_CHANNEL_MAX) / rgb_max;

    if (0 == delta) {
        hsv->hue = 0;
        return;
    }

    int16_t hue;

    if (red == rgb_max) {
        hue = 0 + HSV_HUE_SECTOR_SIZE * (green - blue) / delta;
    } else if (green == rgb_max) {
        hue = HSV_HUE_OFFSET_GREEN + HSV_HUE_SECTOR_SIZE * (blue - red) / delta;
    } else {
        hue = HSV_HUE_OFFSET_BLUE + HSV_HUE_SECTOR_SIZE * (red - green) / delta;
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

    return ((value * brightness) / MAX_BRIGHTNESS);
}

#endif /* ENABLE_COLOUR */
