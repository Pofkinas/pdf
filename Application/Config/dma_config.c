/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "dma_config.h"

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
const static sDmaDesc_t g_static_dma_desc_lut[eDma_Last] = {
    [eDma_Ws2812b_1] = {
        .dma = DMA1,
        .enable_clock_fp = LL_AHB1_GRP1_EnableClock,
        .clock = LL_AHB1_GRP1_PERIPH_DMA1,
        .nvic = DMA1_Stream4_IRQn,
        .channel = LL_DMA_CHANNEL_6,
        .stream = LL_DMA_STREAM_4,
        .data_direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
        .mode = LL_DMA_MODE_CIRCULAR,
        .periph_or_src_increment_mode = LL_DMA_PERIPH_NOINCREMENT,
        .mem_or_dest_increment_mode = LL_DMA_MEMORY_INCREMENT,
        .periph_or_src_size = LL_DMA_PDATAALIGN_WORD,
        .mem_or_dest_size = LL_DMA_MDATAALIGN_WORD,
        .priority_level = LL_DMA_PRIORITY_MEDIUM,
        .fifo_mode_fp = LL_DMA_DisableFifoMode,
    }
};

const static sDmaIsActiveFlags_t g_dma_is_active_flags_fp_lut[eDma_Last] = {
    [eDma_Ws2812b_1] = {
        .is_active_tc_flag_fp = LL_DMA_IsActiveFlag_TC4,
        .is_active_ht_flag_fp = LL_DMA_IsActiveFlag_HT4,
        .is_active_te_flag_fp = LL_DMA_IsActiveFlag_TE4
    }
};

const static sDmaClearFlags_t g_dma_clear_flags_fp_lut[eDma_Last] = {
    [eDma_Ws2812b_1] = {
        .clear_tc_flag_fp = LL_DMA_ClearFlag_TC4,
        .clear_ht_flag_fp = LL_DMA_ClearFlag_HT4,
        .clear_te_flag_fp = LL_DMA_ClearFlag_TE4
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

bool DMA_Config_IsCorrectDma (const eDma_t dma) {
    return (dma >= eDma_First) && (dma < eDma_Last);
}

const sDmaDesc_t *DMA_Config_GetDmaDesc (const eDma_t dma) {
    if (!DMA_Config_IsCorrectDma(dma)) {
        return NULL;
    }

    return &g_static_dma_desc_lut[dma];
}

const sDmaIsActiveFlags_t *DMA_Config_GetDmaIsActiveFlagsFp (const eDma_t dma) {
    if (!DMA_Config_IsCorrectDma(dma)) {
        return NULL;
    }

    return &g_dma_is_active_flags_fp_lut[dma];
}

const sDmaClearFlags_t *DMA_Config_GetDmaClearFlagsFp (const eDma_t dma) {
    if (!DMA_Config_IsCorrectDma(dma)) {
        return NULL;
    }

    return &g_dma_clear_flags_fp_lut[dma];
}
