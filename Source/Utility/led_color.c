/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "led_color.h"

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

/* clang-format off */  
const static uint32_t g_static_rgb_value_lut[eLedColor_Last] = {
    [eLedColor_Off] = 0x000000,
    [eLedColor_Red] = 0xFF0000,
    [eLedColor_Green] = 0x00FF00,
    [eLedColor_Blue] = 0x0000FF,
    [eLedColor_Yellow] = 0xFFFF00,
    [eLedColor_Cyan] = 0x00FFFF,
    [eLedColor_Magenta] = 0xFF00FF,
    [eLedColor_White] = 0xFFFFFF
};

const static sLedColorHsv_t g_static_hsv_value_lut[eLedColor_Last] = {
    [eLedColor_Off] = {
        .hue = 0,
        .saturation = 0,
        .value = 0
    },
    [eLedColor_Red] = {
        .hue = 0,
        .saturation = 255,
        .value = 255
    },
    [eLedColor_Green] = {
        .hue = 85,
        .saturation = 255,
        .value = 255
    },
    [eLedColor_Blue] = {
        .hue = 171,
        .saturation = 255,
        .value = 255
    },
    [eLedColor_Yellow] = {
        .hue = 43,
        .saturation = 255,
        .value = 255
    },
    [eLedColor_Cyan] = {
        .hue = 128,
        .saturation = 255,
        .value = 255
    },
    [eLedColor_Magenta] = {
        .hue = 213,
        .saturation = 255,
        .value = 255
    },
    [eLedColor_White] = {
        .hue = 0,
        .saturation = 0,
        .value = 255
    }
};
/* clang-format on */ 

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

const sLedColorRgb_t LED_GetColorRgb (const eLedColor_t color) {
    sLedColorRgb_t led_color = {0};
    
    if ((color < eLedColor_First) || (color >= eLedColor_Last)) {
        return led_color;
    }

    led_color.color = g_static_rgb_value_lut[color];

    return led_color;
}

const sLedColorHsv_t LED_GetColorHsv (const eLedColor_t color) {
    sLedColorHsv_t led_color = {0};
    
    if ((color < eLedColor_First) || (color >= eLedColor_Last)) {
        return led_color;
    }

    led_color = g_static_hsv_value_lut[color];

    return led_color;
}

void LED_HsvToRgb(const sLedColorHsv_t hsv, sLedColorRgb_t *rgb) {
    if (rgb == NULL) {
        return;
    }
    
    uint8_t hue = hsv.hue;
    uint8_t saturation = hsv.saturation;
    uint8_t value = hsv.value;

    uint8_t red, green, blue;

    if (saturation == 0) {
        red = value;
        green = value;
        blue = value;
    } else {
        uint8_t region = hue / 43;
        uint8_t remainder = (hue - region * 43) * 6;

        uint8_t p = (value * (255 - saturation)) >> 8;
        uint8_t q = (value * (255 - ((saturation * remainder) >> 8))) >> 8;
        uint8_t t = (value * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

        switch (region) {
            case 0: {
                red = value;
                green = t;
                blue = p;
            } break;
            case 1: {
                red = q;
                green = value;
                blue = p;
            } break;
            case 2: {
                red = p;
                green = value;
                blue = t;
            } break;
            case 3: {
                red = p;
                green = q;
                blue = value;
            } break;
            case 4: {
                red = t;
                green = p;
                blue = value;
            } break;
            default: {
                red = value;
                green = p;
                blue = q;
            } break;
        }
    }

    rgb->color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;

    return;
}

void LED_RgbToHsv(const sLedColorRgb_t rgb, sLedColorHsv_t *hsv) {
    if (hsv == NULL) {
        return;
    }

    uint8_t red = (rgb.color >> 16) & 0xFF;
    uint8_t green = (rgb.color >> 8) & 0xFF;
    uint8_t blue = rgb.color & 0xFF;

    uint8_t rgb_min = red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);
    uint8_t rgb_max = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
    
    uint8_t delta = rgb_max - rgb_min;

    hsv->value = rgb_max;

    if (rgb_max == 0) {
        hsv->saturation = 0;
        hsv->hue = 0;
        return;
    }

    hsv->saturation = (delta * 255) / rgb_max;

    if (delta == 0) {
        hsv->hue = 0;
        return;
    }

    int16_t hue;

    if (rgb_max == red) {
        hue = 0 + 43 * (green - blue) / delta;
    } else if (rgb_max == green) {
        hue = 85 + 43 * (blue - red) / delta;
    } else {
        hue = 171 + 43 * (red - green) / delta;
    }

    if (hue < 0) hue += 256;

    hsv->hue = (uint8_t)hue;

    return;
}

uint8_t LED_ScaleBrightness (const uint8_t value, const uint8_t brightness) {
    if (brightness == 0) {
        return 0;
    }

    if (brightness > MAX_BRIGHTNESS) {
        return value;
    }

    return (value * brightness) / MAX_BRIGHTNESS;
}
