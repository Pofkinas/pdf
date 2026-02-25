#ifndef SOURCE_DRIVER_DMA_DRIVER_H_
#define SOURCE_DRIVER_DMA_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_DMA)
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "dma_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eDma_Flags {
    eDma_Flags_First = 0,
    eDma_Flags_TC = eDma_Flags_First,
    eDma_Flags_HT,
    eDma_Flags_TE,
    eDma_Flags_Last
} eDma_Flags_t;


typedef struct sDmaInit {
    eDma_t stream;
    uint32_t *periph_or_src_addr;
    uint32_t *mem_or_dest_addr;
    uint16_t data_buffer_size;
    void (*isr_callback) (void *isr_callback_context, const eDma_Flags_t flag);
    void *isr_callback_context;
} sDmaInit_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool DMA_Driver_Init (sDmaInit_t *data);
bool DMA_Driver_ConfigureStream (const eDma_t stream, uint32_t *src_address, uint32_t *dst_address, const size_t size);
bool DMA_Driver_EnableStream (const eDma_t stream);
bool DMA_Driver_DisableStream (const eDma_t stream);
bool DMA_Driver_ClearFlag (const eDma_t stream, const eDma_Flags_t flag);
bool DMA_Driver_ClearAllFlags (const eDma_t stream);
bool DMA_Driver_EnableIt (const eDma_t stream, const eDma_Flags_t flag);
bool DMA_Driver_EnableItAll (const eDma_t stream);
bool DMA_Driver_DisableIt (const eDma_t stream, const eDma_Flags_t flag);
bool DMA_Driver_DisableItAll (const eDma_t stream);

#endif /* ENABLE_DMA */
#endif /* SOURCE_DRIVER_DMA_DRIVER_H_ */
