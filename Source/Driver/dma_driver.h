#ifndef SOURCE_DRIVER_DMA_DRIVER_H_
#define SOURCE_DRIVER_DMA_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "framework_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eDmaDriver{
    eDmaDriver_First = 0,
    
    #ifdef USE_WS2812B_1
    eDmaDriver_Ws2812b_1,
    #endif

    #ifdef USE_WS2812B_2
    eDmaDriver_Ws2812b_2,
    #endif

    eDmaDriver_Last
} eDmaDriver_t;

typedef enum eDmaDriver_Flags {
    eDmaDriver_Flags_First = 0,
    eDmaDriver_Flags_TC = eDmaDriver_Flags_First,
    eDmaDriver_Flags_HT,
    eDmaDriver_Flags_TE,
    eDmaDriver_Flags_Last
} eDmaDriver_Flags_t;

typedef struct sDmaInit {
    eDmaDriver_t stream;
    uint32_t *periph_or_src_addr;
    uint32_t *mem_or_dest_addr;
    uint16_t data_buffer_size;
    void (*isr_callback) (void *isr_callback_context, const eDmaDriver_Flags_t);
    void *isr_callback_context;
} sDmaInit_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void DMA_Driver_DefinePerips(const sDmaStaticDesc_t *dma_lut, const sDmaIsActiveFlags_t *is_active_flags_fp_lut, const sDmaClearFlags_t *clear_flags_fp_lut);
bool DMA_Driver_Init(sDmaInit_t *data);
bool DMA_Driver_ConfigureStream (const eDmaDriver_t stream, uint32_t *src_address, uint32_t *dst_address, const size_t size);
bool DMA_Driver_EnableStream(const eDmaDriver_t stream);
bool DMA_Driver_DisableStream(const eDmaDriver_t stream);
bool DMA_Driver_ClearFlag (const eDmaDriver_t stream, const eDmaDriver_Flags_t flag);
bool DMA_Driver_ClearAllFlags (const eDmaDriver_t stream);
bool DMA_Driver_EnableIt (const eDmaDriver_t stream, const eDmaDriver_Flags_t flag);
bool DMA_Driver_EnableItAll (const eDmaDriver_t stream);
bool DMA_Driver_DisableIt (const eDmaDriver_t stream, const eDmaDriver_Flags_t flag);
bool DMA_Driver_DisableItAll (const eDmaDriver_t stream);

#endif /* SOURCE_DRIVER_DMA_DRIVER_H_ */
