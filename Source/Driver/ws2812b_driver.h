#ifndef SOURCE_DRIVER_WS2812B_DRIVER_H_
#define SOURCE_DRIVER_WS2812B_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_WS2812B)
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "ws2812b_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eLedTransferState {
    eLedTransferState_First = 0,
    eLedTransferState_Start = eLedTransferState_First,
    eLedTransferState_Complete,
    eLedTransferState_TransferError,
    eLedTransferState_Last
} eLedTransferState_t;

typedef void (*led_driver_callback_t) (void *context, const eLedTransferState_t transfer_state);

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool WS2812B_Driver_Init (const eWs2812b_t device, led_driver_callback_t callback, void *callback_context);
bool WS2812B_Driver_Set (const eWs2812b_t device, uint8_t *led_data, size_t led_count);
bool WS2812B_Driver_Reset (const eWs2812b_t device);
uint16_t WS2812B_Driver_GetMinRefreshRate (const eWs2812b_t device);

#endif /* ENABLE_WS2812B */
#endif /* SOURCE_DRIVER_WS2812B_DRIVER_H_ */
