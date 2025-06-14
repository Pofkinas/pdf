/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "ws2812b_driver.h"

#ifdef USE_WS2812B

#include <string.h>
#include <math.h>
#include "timer_driver.h"
#include "pwm_driver.h"
#include "dma_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define BYTE 8
#define LED_RESOLUTION 2
#define BITS_PER_LED (LED_DATA_CHANNELS * BYTE)
#define WS2812B_DMA_BUFFER_HALF_SIZE  (LED_RESOLUTION * BITS_PER_LED)
#define WS2812B_DMA_BUFFER_SIZE  (2 * WS2812B_DMA_BUFFER_HALF_SIZE)

#define SINGLE_DATA_TRANSFER_TIME_NS 1250
#define DATA_TRANSFER_HIGH_TIME (850.0f / SINGLE_DATA_TRANSFER_TIME_NS)
#define DATA_TRANSFER_LOW_TIME (400.0f / SINGLE_DATA_TRANSFER_TIME_NS)

#define LATCH_LED_TRANSFERS 2

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eWs2812bDriver_State {
    eWs2812bDriverState_First = 0,
    eWs2812bDriverState_Idle = eWs2812bDriverState_First,
    eWs2812bDriverState_Transfer,
    eWs2812bDriverState_Latch,
    eWs2812bDriverState_Reset,
    eWs2812bDriverState_Last
} eWs2812bDriver_State_t;

typedef enum eDmaBuffer_State {
    eDmaBuffer_State_First = 0,
    eDmaBuffer_State_Empty = eDmaBuffer_State_First,
    eDmaBuffer_State_FirstHalfEmpty,
    eDmaBuffer_State_SecondHalfEmpty,
    eDmaBuffer_State_Last
} eDmaBuffer_State_t;

typedef struct sWs2812bStaticDesc {
    eTimerDriver_t timer;
    ePwmDevice_t pwm_device;
    eDmaDriver_t dma_stream;
    size_t total_led;
} sWs2812bStaticDesc_t;

typedef struct sWs2812bDynamicDesc {
    eWs2812bDriver_t device;
    bool is_init;
    eWs2812bDriver_State_t state;
    eDmaBuffer_State_t dma_buffer_state;
    uint8_t *led_data;
    size_t led_to_set;
    size_t processed_led;
    size_t sent_led_count;
    uint32_t dma_buffer[WS2812B_DMA_BUFFER_SIZE];
    uint32_t dma_buffer_reset[WS2812B_DMA_BUFFER_SIZE];
    void (*led_driver_callback) (void *context, const eLedTransferState_t transfer_state);
    void *callback_context;
    uint8_t high_time;
    uint8_t low_time;
} sWs2812bDynamicDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

const static uint32_t g_ws2812b_latch_data[WS2812B_DMA_BUFFER_SIZE] = {0};
const static uint8_t g_led_order_grb[3] = {1, 0, 2};

/* clang-format off */
const static sWs2812bStaticDesc_t g_static_ws2812b_lut[eWs2812bDriver_Last] = {
    #ifdef USE_WS2812B_1
    [eWs2812bDriver_1] = {
        .timer = eTimerDriver_TIM5,
        .pwm_device = ePwmDevice_Ws2812b_1,
        .dma_stream = eDmaDriver_Ws2812b_1,
        .total_led = WS2812B_1_LED_COUNT
    },
    #endif

    #ifdef USE_WS2812B_2
    [eWs2812bDriver_2] = {
        .timer = eTimerDriver_TIM5,
        .pwm_device = ePwmDevice_Ws2812b_2,
        .dma_stream = eDmaDriver_Ws2812b_2,
        .total_led = WS2812B_2_LED_COUNT
    },
    #endif
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/* clang-format off */
static sWs2812bDynamicDesc_t g_dynamic_ws2812b_lut[eWs2812bDriver_Last] = {
    #ifdef USE_WS2812B_1
    [eWs2812bDriver_1] = {
        .is_init = false,
        .state = eWs2812bDriverState_Idle,
        .dma_buffer_state = eDmaBuffer_State_Empty,
        .led_data = NULL,
        .led_to_set = 0,
        .processed_led = 0,
        .sent_led_count = 0,
        .led_driver_callback = NULL,
        .high_time = 0,
        .low_time = 0
    },
    #endif

    #ifdef USE_WS2812B_2
    [eWs2812bDriver_2] = {
        .is_init = false,
        .state = eWs2812bDriverState_Idle,
        .dma_buffer_state = eDmaBuffer_State_Empty,
        .led_data = NULL,
        .led_to_set = 0,
        .processed_led = 0,
        .sent_led_count = 0,
        .led_driver_callback = NULL,
        .high_time = 0,
        .low_time = 0
    },
    #endif
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void WS2812B_Driver_Dma_ISRHandler (void *isr_callback_contex, const eDmaDriver_Flags_t flag);
static bool WS2812B_Driver_IsAllLedDataTransfered (const eWs2812bDriver_t device);
static void WS2812B_Driver_ProcessDmaBuffer (const eWs2812bDriver_t device);
static void WS2812B_Driver_Latch (const eWs2812bDriver_t device);
static void WS2812B_Driver_Stop (const eWs2812bDriver_t device);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void WS2812B_Driver_Dma_ISRHandler (void *isr_callback_context, const eDmaDriver_Flags_t flag) {
    if (isr_callback_context == NULL) {
        return;
    }
    
    sWs2812bDynamicDesc_t *context = (sWs2812bDynamicDesc_t *) isr_callback_context;

    if ((flag < eDmaDriver_Flags_First) || (flag >= eDmaDriver_Flags_Last)) {
        return;
    }

    DMA_Driver_ClearFlag(g_static_ws2812b_lut[context->device].dma_stream, flag);

    if (flag == eDmaDriver_Flags_TE) {
        g_dynamic_ws2812b_lut[context->device].led_driver_callback(g_dynamic_ws2812b_lut[context->device].callback_context, eLedTransferState_TransferError);

        WS2812B_Driver_Reset(context->device);
        
        return;
    }

    g_dynamic_ws2812b_lut[context->device].sent_led_count += LED_RESOLUTION;
            
    switch (g_dynamic_ws2812b_lut[context->device].state) {
        case eWs2812bDriverState_Transfer: {
            g_dynamic_ws2812b_lut[context->device].dma_buffer_state = (flag == eDmaDriver_Flags_TC) ? eDmaBuffer_State_SecondHalfEmpty : eDmaBuffer_State_FirstHalfEmpty;

            if (!WS2812B_Driver_IsAllLedDataTransfered(context->device)) {
                WS2812B_Driver_ProcessDmaBuffer(context->device);

                return;
            }

            WS2812B_Driver_Latch(context->device);

            g_dynamic_ws2812b_lut[context->device].state = eWs2812bDriverState_Latch;         
        } break;
        case eWs2812bDriverState_Latch: {
            if (WS2812B_Driver_IsAllLedDataTransfered(context->device)) {
                WS2812B_Driver_Stop(context->device);
            }
        } break;
        case eWs2812bDriverState_Reset: {
            if (WS2812B_Driver_IsAllLedDataTransfered(context->device)) {
                WS2812B_Driver_Latch(context->device);

                g_dynamic_ws2812b_lut[context->device].state = eWs2812bDriverState_Latch;
            }
        } break;
        default: {
            break;
        }
    }

    return;
}

static bool WS2812B_Driver_IsAllLedDataTransfered (const eWs2812bDriver_t device) {
    if ((device <= eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return false;
    }

    return (g_dynamic_ws2812b_lut[device].sent_led_count >= g_dynamic_ws2812b_lut[device].led_to_set);
}

static void WS2812B_Driver_ProcessDmaBuffer (const eWs2812bDriver_t device) {
    if ((device <= eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return;
    }

    if (!g_dynamic_ws2812b_lut[device].is_init) {
        return;
    }

    uint32_t *dma_buffer = g_dynamic_ws2812b_lut[device].dma_buffer;
    uint8_t *led_data = g_dynamic_ws2812b_lut[device].led_data + (g_dynamic_ws2812b_lut[device].processed_led * LED_DATA_CHANNELS);
    size_t leds_to_fill = LED_RESOLUTION;

    switch (g_dynamic_ws2812b_lut[device].dma_buffer_state) {
        case eDmaBuffer_State_Empty: {
            leds_to_fill = leds_to_fill * 2;
        } break;
        case eDmaBuffer_State_FirstHalfEmpty: {
        } break;
        case eDmaBuffer_State_SecondHalfEmpty: {
            dma_buffer += WS2812B_DMA_BUFFER_HALF_SIZE;
        } break;
        default: {
            return;
        } 
    }

    size_t led_bit_offset = 0;
    uint8_t led_data_map = 0;
    size_t dma_buffer_offset = 0;

    for (size_t led = 0; led < leds_to_fill; led++) {
        led_bit_offset = led * BITS_PER_LED;

        if (g_dynamic_ws2812b_lut[device].led_to_set == g_dynamic_ws2812b_lut[device].processed_led) {
            memset(dma_buffer + led_bit_offset, 0, (((g_dynamic_ws2812b_lut[device].dma_buffer_state == eDmaBuffer_State_SecondHalfEmpty) ? WS2812B_DMA_BUFFER_HALF_SIZE : WS2812B_DMA_BUFFER_SIZE) - led_bit_offset) * sizeof(uint32_t));

            return;
        }

        for (uint8_t channel = 0; channel < LED_DATA_CHANNELS; channel++) {
            led_data_map = g_led_order_grb[channel];
            dma_buffer_offset = led_bit_offset + channel * BYTE;

            for (uint8_t bit = 0; bit < BYTE; bit++) {
                dma_buffer[dma_buffer_offset + bit] = ((led_data[led_data_map] >> (7 - bit)) & 1) ? g_dynamic_ws2812b_lut[device].high_time : g_dynamic_ws2812b_lut[device].low_time;                
            }
        }

        led_data += LED_DATA_CHANNELS;
        g_dynamic_ws2812b_lut[device].processed_led++;
    }

    return;
}

static void WS2812B_Driver_Latch (const eWs2812bDriver_t device) {
    if ((device < eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return;
    }

    g_dynamic_ws2812b_lut[device].sent_led_count = 0;
    g_dynamic_ws2812b_lut[device].led_to_set = LATCH_LED_TRANSFERS;

    DMA_Driver_DisableStream(g_static_ws2812b_lut[device].dma_stream);
    DMA_Driver_ConfigureStream(g_static_ws2812b_lut[device].dma_stream, (uint32_t *) g_ws2812b_latch_data, NULL, WS2812B_DMA_BUFFER_SIZE);
    DMA_Driver_EnableStream(g_static_ws2812b_lut[device].dma_stream);

    return;
}

static void WS2812B_Driver_Stop (const eWs2812bDriver_t device) {
    if ((device < eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return;
    }

    PWM_Driver_Disable_Device(g_static_ws2812b_lut[device].pwm_device);
    DMA_Driver_DisableStream(g_static_ws2812b_lut[device].dma_stream);
    DMA_Driver_ClearAllFlags(g_static_ws2812b_lut[device].dma_stream);

    g_dynamic_ws2812b_lut[device].led_driver_callback(g_dynamic_ws2812b_lut[device].callback_context, eLedTransferState_Complete);

    g_dynamic_ws2812b_lut[device].dma_buffer_state = eDmaBuffer_State_Empty;
    g_dynamic_ws2812b_lut[device].state = eWs2812bDriverState_Idle;

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool WS2812B_Driver_Init (const eWs2812bDriver_t device, led_driver_callback_t callback, void *callback_context) {
    if ((device <= eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return false;
    }

    if (callback == NULL) {
        return false;
    }

    if (callback_context == NULL) {
        return false;
    }
    
    if (g_dynamic_ws2812b_lut[device].is_init) {
        return true;
    }

    sDmaInit_t dma_init_struct = {
        .stream = g_static_ws2812b_lut[device].dma_stream,
        .periph_or_src_addr = (uint32_t*) PWM_Driver_GetRegAddr(g_static_ws2812b_lut[device].pwm_device),
        .mem_or_dest_addr = g_dynamic_ws2812b_lut[device].dma_buffer,
        .data_buffer_size = WS2812B_DMA_BUFFER_SIZE,
        .isr_callback = &WS2812B_Driver_Dma_ISRHandler,
        .isr_callback_context = &g_dynamic_ws2812b_lut[device]
    };
        
    if (!DMA_Driver_Init(&dma_init_struct)) {
        return false;
    }

    g_dynamic_ws2812b_lut[device].high_time = (uint8_t) (DATA_TRANSFER_HIGH_TIME * Timer_Driver_GetResolution(g_static_ws2812b_lut[device].timer));
    g_dynamic_ws2812b_lut[device].low_time = (uint8_t) (DATA_TRANSFER_LOW_TIME * Timer_Driver_GetResolution(g_static_ws2812b_lut[device].timer));

    for (size_t led_bit = 0; led_bit < WS2812B_DMA_BUFFER_SIZE; led_bit++) {
        g_dynamic_ws2812b_lut[device].dma_buffer_reset[led_bit] = g_dynamic_ws2812b_lut[device].low_time;
    }

    g_dynamic_ws2812b_lut[device].led_driver_callback = callback;
    g_dynamic_ws2812b_lut[device].callback_context = callback_context;
    g_dynamic_ws2812b_lut[device].device = device;
        
    g_dynamic_ws2812b_lut[device].is_init = true;

    return true;
}

bool WS2812B_Driver_Set (const eWs2812bDriver_t device, uint8_t *led_data, size_t led_count) {
    if ((device <= eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return false;
    }

    if (led_data == NULL) {
        return false;
    }

    if (led_count == 0 || led_count > g_static_ws2812b_lut[device].total_led) {
        return false;
    }

    if (!g_dynamic_ws2812b_lut[device].is_init) {
        return false;
    }

    if (g_dynamic_ws2812b_lut[device].state != eWs2812bDriverState_Idle) {
        return false;
    }

    g_dynamic_ws2812b_lut[device].led_data = led_data;
    g_dynamic_ws2812b_lut[device].led_to_set = led_count;
    g_dynamic_ws2812b_lut[device].processed_led = 0;
    g_dynamic_ws2812b_lut[device].sent_led_count = 0;

    if (!DMA_Driver_ConfigureStream(g_static_ws2812b_lut[device].dma_stream, g_dynamic_ws2812b_lut[device].dma_buffer, NULL, WS2812B_DMA_BUFFER_SIZE)) {
        return false;
    }
    
    memset(g_dynamic_ws2812b_lut[device].dma_buffer, 0, WS2812B_DMA_BUFFER_SIZE * sizeof(uint32_t));

    g_dynamic_ws2812b_lut[device].dma_buffer_state = eDmaBuffer_State_Empty;

    WS2812B_Driver_ProcessDmaBuffer(device);

    if (!DMA_Driver_ClearAllFlags(g_static_ws2812b_lut[device].dma_stream)) {
        return false;
    }

    if (!DMA_Driver_EnableItAll(g_static_ws2812b_lut[device].dma_stream)) {
        return false;
    }

    if (!DMA_Driver_EnableStream(g_static_ws2812b_lut[device].dma_stream)) {
        return false;
    }

    if (!PWM_Driver_Enable_Device(g_static_ws2812b_lut[device].pwm_device)) {
        return false;
    }

    g_dynamic_ws2812b_lut[device].state = eWs2812bDriverState_Transfer;

    return true;
}

bool WS2812B_Driver_Reset (const eWs2812bDriver_t device) {
    if ((device <= eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return false;
    }

    if (!g_dynamic_ws2812b_lut[device].is_init) {
        return false;
    }

    if (g_dynamic_ws2812b_lut[device].state != eWs2812bDriverState_Idle) {
        return false;
    }

    g_dynamic_ws2812b_lut[device].led_to_set = g_static_ws2812b_lut[device].total_led;
    g_dynamic_ws2812b_lut[device].sent_led_count = 0;
    
    if (!DMA_Driver_ConfigureStream(g_static_ws2812b_lut[device].dma_stream, g_dynamic_ws2812b_lut[device].dma_buffer_reset, NULL, WS2812B_DMA_BUFFER_SIZE)) {
        return false;
    }

    if (!DMA_Driver_ClearAllFlags(g_static_ws2812b_lut[device].dma_stream)) {
        return false;
    }

    if (!DMA_Driver_EnableItAll(g_static_ws2812b_lut[device].dma_stream)) {
        return false;
    }

    if (!DMA_Driver_EnableStream(g_static_ws2812b_lut[device].dma_stream)) {
        return false;
    }

    if (!PWM_Driver_Enable_Device(g_static_ws2812b_lut[device].pwm_device)) {
        return false;
    }

    g_dynamic_ws2812b_lut[device].state = eWs2812bDriverState_Reset;

    return true;
}

uint16_t WS2812B_Driver_GetMinRefreshRate (const eWs2812bDriver_t device) {
    if ((device <= eWs2812bDriver_First) || (device >= eWs2812bDriver_Last)) {
        return 0;
    }

    if (!g_dynamic_ws2812b_lut[device].is_init) {
        return 0;
    }

    float transfer_time_ms = SINGLE_DATA_TRANSFER_TIME_NS * BITS_PER_LED * (g_static_ws2812b_lut[device].total_led + LATCH_LED_TRANSFERS) / 10000000;

    if (transfer_time_ms < 1.0f) {
        return 1;
    } else {
        return (uint16_t) ceilf(transfer_time_ms);
    }
}

#endif
