#ifndef SOURCE_UTILITY_COLOUR_H_
#define SOURCE_UTILITY_COLOUR_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_COLOUR)
#include <stdbool.h>
#include <stdint.h>
#include <colour_config.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/// Bit positions of R, G, B bytes in a packed 24-bit ColourRgb_t value.
#define RGB_RED_SHIFT 16U
#define RGB_GREEN_SHIFT 8U
#define RGB_BLUE_SHIFT 0U
#define RGB_BYTE_MASK 0xFF

#define CHANNEL_MAX 255U

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void Colour_HsvToRgb (const sColourHsv_t hsv, ColourRgb_t *rgb);
void Colour_RgbToHsv (const ColourRgb_t rgb, sColourHsv_t *hsv);
uint8_t Colour_ScaleBrightness (const uint8_t value, const uint8_t brightness);

#endif /* ENABLE_COLOUR */
#endif /* SOURCE_UTILITY_COLOUR_H_ */
