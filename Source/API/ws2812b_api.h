#ifndef SOURCE_API_WS2812B_API_H_
#define SOURCE_API_WS2812B_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_WS2812B)
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "ws2812b_config.h"
#include "colour.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eLedAnimation {
    eLedAnimation_First = 0,
    eLedAnimation_SolidColour = eLedAnimation_First,
    eLedAnimation_SegmentFill,
    eLedAnimation_Rainbow,
    eLedAnimation_Last
} eLedAnimation_t;

typedef enum eDirection {
    eDirection_First = 0,
    eDirection_Up = eDirection_First,
    eDirection_Down,
    eDirection_Last
} eDirection_t;

typedef struct sLedAnimationDesc {
    eWs2812b_t device;
    eLedAnimation_t animation;
    uint8_t brightness;
    void *data;
} sLedAnimationDesc_t;

typedef struct sLedAnimationInstance {
    void *context;
    void (*build_animation)(void *context);
    void (*free_animation)(void *context);
} sLedAnimationInstance_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool WS2812B_API_InitAll (void);
bool WS2812B_API_AddAnimation (sLedAnimationDesc_t *animation_data);
bool WS2812B_API_ClearAnimations (const eWs2812b_t device);
bool WS2812B_API_Start (const eWs2812b_t device);
bool WS2812B_API_Stop (const eWs2812b_t device);
bool WS2812B_API_Reset (const eWs2812b_t device);
bool WS2812B_API_FreeData (void *data);
uint32_t WS2812B_API_GetLedCount (const eWs2812b_t device);
bool WS2812B_API_SetColour (const eWs2812b_t device, size_t led_number, const uint8_t red, const uint8_t green, const uint8_t blue);
bool WS2812B_API_FillColour (const eWs2812b_t device, const uint8_t red, const uint8_t green, const uint8_t blue);
bool WS2812B_API_FillSegment (const eWs2812b_t device, const size_t start_led, const size_t end_led, const uint8_t red, const uint8_t green, const uint8_t blue);

#endif /* ENABLE_WS2812B */
#endif /* SOURCE_API_WS2812B_API_H_ */
