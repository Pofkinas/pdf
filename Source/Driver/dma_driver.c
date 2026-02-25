/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "dma_driver.h"

#if defined(ENABLE_DMA)

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sDmaDynamicDesc {
    bool is_init;
    uint32_t *periph_or_src_addr;
    uint32_t *mem_or_dest_addr;
    void (*isr_callback) (void *isr_callback_context, const eDma_Flags_t);
    void *isr_callback_context;
} sDmaDynamicDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sDmaDesc_t g_dma_desc_lut[eDma_Last] = {0};
static sDmaIsActiveFlags_t g_dma_is_active_flags_fp_lut[eDma_Last] = {0};
static sDmaClearFlags_t g_dma_clear_flags_fp_lut[eDma_Last] = {0};
static sDmaDynamicDesc_t g_dynamic_dma_lut[eDma_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool DMA_Driver_IsCorrectFlag (const eDma_Flags_t flag);
static void DMAx_Streamx_ISRHandler(const eDma_t stream, const eDma_Flags_t flag);
void DMA1_Stream0_IRQHandler (void);
void DMA1_Stream1_IRQHandler (void);
void DMA1_Stream2_IRQHandler (void);
void DMA1_Stream3_IRQHandler (void);
void DMA1_Stream4_IRQHandler (void);
void DMA1_Stream5_IRQHandler (void);
void DMA1_Stream6_IRQHandler (void);
void DMA1_Stream7_IRQHandler (void);
void DMA2_Stream0_IRQHandler (void);
void DMA2_Stream1_IRQHandler (void);
void DMA2_Stream2_IRQHandler (void);
void DMA2_Stream3_IRQHandler (void);
void DMA2_Stream4_IRQHandler (void);
void DMA2_Stream5_IRQHandler (void);
void DMA2_Stream6_IRQHandler (void);
void DMA2_Stream7_IRQHandler (void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static bool DMA_Driver_IsCorrectFlag (const eDma_Flags_t flag) {
    return (flag >= eDma_Flags_First) && (flag < eDma_Flags_Last);
}

static void DMAx_Streamx_ISRHandler (const eDma_t stream, const eDma_Flags_t flag) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return;
    }

    if (!DMA_Driver_IsCorrectFlag(flag)) {
        return;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return;
    }

    if (NULL == g_dynamic_dma_lut[stream].isr_callback) {
        return;
    }

    DMA_Driver_ClearFlag(stream, flag);

    g_dynamic_dma_lut[stream].isr_callback(g_dynamic_dma_lut[stream].isr_callback_context, flag);

    return;
}

void DMA1_Stream0_IRQHandler (void) {
    #if defined(DMA_1_STREAM_0)
    if (LL_DMA_IsActiveFlag_TC0(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_0, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT0(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_0, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE0(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_0, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_0 */

    return;
}

void DMA1_Stream1_IRQHandler (void) {
    #if defined(DMA_1_STREAM_1)
    if (LL_DMA_IsActiveFlag_TC1(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_1, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT1(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_1, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE1(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_1, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_1 */

    return;
}

void DMA1_Stream2_IRQHandler (void) {
    #if defined(DMA_1_STREAM_2)
    if (LL_DMA_IsActiveFlag_TC2(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_2, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT2(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_2, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE2(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_2, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_2 */

    return;
}

void DMA1_Stream3_IRQHandler (void) {
    #if defined(DMA_1_STREAM_3)
    if (LL_DMA_IsActiveFlag_TC3(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_3, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT3(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_3, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE3(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_3, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_3 */

    return;
}

void DMA1_Stream4_IRQHandler (void) {
    #if defined(DMA_1_STREAM_4)
    if (LL_DMA_IsActiveFlag_TC4(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_4, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT4(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_4, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE4(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_4, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_4 */

    return;
}

void DMA1_Stream5_IRQHandler (void) {
    #if defined(DMA_1_STREAM_5)
    if (LL_DMA_IsActiveFlag_TC5(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_5, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT5(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_5, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE5(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_5, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_5 */

    return;
}

void DMA1_Stream6_IRQHandler (void) {
    #if defined(DMA_1_STREAM_6)
    if (LL_DMA_IsActiveFlag_TC6(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_6, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT6(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_6, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE6(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_6, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_6 */

    return;
}

void DMA1_Stream7_IRQHandler (void) {
    #if defined(DMA_1_STREAM_7)
    if (LL_DMA_IsActiveFlag_TC7(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_7, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT7(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_7, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE7(DMA1)) {
        DMAx_Streamx_ISRHandler(DMA_1_STREAM_7, eDma_Flags_TE);
    }
    #endif /* DMA_1_STREAM_7 */

    return;
}

void DMA2_Stream0_IRQHandler (void) {
    #if defined(DMA_2_STREAM_0)
    if (LL_DMA_IsActiveFlag_TC0(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_0, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT0(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_0, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE0(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_0, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_0 */

    return;
}

void DMA2_Stream1_IRQHandler (void) {
    #if defined(DMA_2_STREAM_1)
    if (LL_DMA_IsActiveFlag_TC1(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_1, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT1(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_1, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE1(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_1, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_1 */

    return;
}

void DMA2_Stream2_IRQHandler (void) {
    #if defined(DMA_2_STREAM_2)
    if (LL_DMA_IsActiveFlag_TC2(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_2, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT2(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_2, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE2(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_2, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_2 */

    return;
}

void DMA2_Stream3_IRQHandler (void) {
    #if defined(DMA_2_STREAM_3)
    if (LL_DMA_IsActiveFlag_TC3(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_3, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT3(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_3, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE3(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_3, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_3 */

    return;
}

void DMA2_Stream4_IRQHandler (void) {
    #if defined(DMA_2_STREAM_4)
    if (LL_DMA_IsActiveFlag_TC4(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_4, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT4(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_4, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE4(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_4, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_4 */

    return;
}

void DMA2_Stream5_IRQHandler (void) {
    #if defined(DMA_2_STREAM_5)
    if (LL_DMA_IsActiveFlag_TC5(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_5, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT5(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_5, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE5(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_5, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_5 */

    return;
}

void DMA2_Stream6_IRQHandler (void) {
    #if defined(DMA_2_STREAM_6)
    if (LL_DMA_IsActiveFlag_TC6(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_6, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT6(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_6, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE6(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_6, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_6 */

    return;
}

void DMA2_Stream7_IRQHandler (void) {
    #if defined(DMA_2_STREAM_7)
    if (LL_DMA_IsActiveFlag_TC7(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_7, eDma_Flags_TC);
    }

    if (LL_DMA_IsActiveFlag_HT7(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_7, eDma_Flags_HT);
    }

    if (LL_DMA_IsActiveFlag_TE7(DMA2)) {
        DMAx_Streamx_ISRHandler(DMA_2_STREAM_7, eDma_Flags_TE);
    }
    #endif /* DMA_2_STREAM_7 */

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool DMA_Driver_Init (sDmaInit_t *data) {
    if (NULL == data) {
        return false;
    }

    if (!DMA_Config_IsCorrectDma(data->stream)) {
        return false;
    }

    if (g_dynamic_dma_lut[data->stream].is_init) {
        return true;
    }

    const sDmaDesc_t *desc = DMA_Config_GetDmaDesc(data->stream);
    const sDmaIsActiveFlags_t *is_active_flags_fp = DMA_Config_GetDmaIsActiveFlagsFp(data->stream);
    const sDmaClearFlags_t *clear_flags_fp = DMA_Config_GetDmaClearFlagsFp(data->stream);

    if ((NULL == desc) || (NULL == is_active_flags_fp) || (NULL == clear_flags_fp)) {
        return false;
    }

    g_dma_desc_lut[data->stream] = *desc;
    g_dma_is_active_flags_fp_lut[data->stream] = *is_active_flags_fp;
    g_dma_clear_flags_fp_lut[data->stream] = *clear_flags_fp;

    g_dma_desc_lut[data->stream].enable_clock_fp(g_dma_desc_lut[data->stream].clock);
    
    LL_DMA_InitTypeDef dma_init_struct = {0};

    dma_init_struct.PeriphOrM2MSrcAddress = (uint32_t) data->periph_or_src_addr;
    dma_init_struct.MemoryOrM2MDstAddress = (uint32_t) data->mem_or_dest_addr;
    dma_init_struct.Direction = g_dma_desc_lut[data->stream].data_direction;
    dma_init_struct.Mode = g_dma_desc_lut[data->stream].mode;
    dma_init_struct.PeriphOrM2MSrcIncMode = g_dma_desc_lut[data->stream].periph_or_src_increment_mode;
    dma_init_struct.MemoryOrM2MDstIncMode = g_dma_desc_lut[data->stream].mem_or_dest_increment_mode;
    dma_init_struct.PeriphOrM2MSrcDataSize = g_dma_desc_lut[data->stream].periph_or_src_size;
    dma_init_struct.MemoryOrM2MDstDataSize = g_dma_desc_lut[data->stream].mem_or_dest_size;
    dma_init_struct.NbData = data->data_buffer_size;
    dma_init_struct.Channel = g_dma_desc_lut[data->stream].channel;
    dma_init_struct.Priority = g_dma_desc_lut[data->stream].priority_level;

    LL_DMA_DisableStream(g_dma_desc_lut[data->stream].dma, g_dma_desc_lut[data->stream].stream);

    if (ERROR == LL_DMA_Init(g_dma_desc_lut[data->stream].dma, g_dma_desc_lut[data->stream].stream, &dma_init_struct)) {
        return false;
    }

    g_dma_desc_lut[data->stream].fifo_mode_fp(g_dma_desc_lut[data->stream].dma, g_dma_desc_lut[data->stream].stream);

    if (NULL != data->isr_callback) {
        g_dynamic_dma_lut[data->stream].isr_callback = data->isr_callback;
        g_dynamic_dma_lut[data->stream].isr_callback_context = data->isr_callback_context;
        
        NVIC_SetPriority(g_dma_desc_lut[data->stream].nvic, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
        NVIC_EnableIRQ(g_dma_desc_lut[data->stream].nvic);
        
        LL_DMA_EnableIT_TC(g_dma_desc_lut[data->stream].dma, g_dma_desc_lut[data->stream].stream);
        LL_DMA_EnableIT_HT(g_dma_desc_lut[data->stream].dma, g_dma_desc_lut[data->stream].stream);
        LL_DMA_EnableIT_TE(g_dma_desc_lut[data->stream].dma, g_dma_desc_lut[data->stream].stream);
    }

    switch (g_dma_desc_lut[data->stream].data_direction) {
        case LL_DMA_DIRECTION_MEMORY_TO_PERIPH: {
            g_dynamic_dma_lut[data->stream].periph_or_src_addr = data->mem_or_dest_addr;
            g_dynamic_dma_lut[data->stream].mem_or_dest_addr = data->periph_or_src_addr;
        } break;
        case LL_DMA_DIRECTION_PERIPH_TO_MEMORY: {
            g_dynamic_dma_lut[data->stream].periph_or_src_addr = data->periph_or_src_addr;
            g_dynamic_dma_lut[data->stream].mem_or_dest_addr = data->mem_or_dest_addr;
        } break;
        default: {
            return false;
        }
    }
    
    g_dynamic_dma_lut[data->stream].is_init = true;

    return true;
}

bool DMA_Driver_ConfigureStream (const eDma_t stream, uint32_t *src_address, uint32_t *dst_address, const size_t size) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    if (NULL != src_address) {
        g_dynamic_dma_lut[stream].periph_or_src_addr = src_address;
    }

    if (NULL != dst_address) {
        g_dynamic_dma_lut[stream].mem_or_dest_addr = dst_address;
    }

    if (0 == size) {
        return false;
    }

    if (LL_DMA_IsEnabledStream(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream)) { 
        DMA_Driver_DisableStream(stream);
    }

    while (LL_DMA_IsEnabledStream(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream)) {} 

    LL_DMA_ConfigAddresses(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream, (uint32_t) g_dynamic_dma_lut[stream].periph_or_src_addr, (uint32_t) g_dynamic_dma_lut[stream].mem_or_dest_addr, g_dma_desc_lut[stream].data_direction);
    LL_DMA_SetDataLength(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream, size);
    LL_DMA_SetChannelSelection(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream, g_dma_desc_lut[stream].channel);

    return true;
}

bool DMA_Driver_EnableStream (const eDma_t stream) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    LL_DMA_EnableStream(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);

    return true;
}

bool DMA_Driver_DisableStream (const eDma_t stream) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    LL_DMA_DisableStream(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);

    while (LL_DMA_IsEnabledStream(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream)) {} 

    return true;
}

bool DMA_Driver_ClearFlag (const eDma_t stream, const eDma_Flags_t flag) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!DMA_Driver_IsCorrectFlag(flag)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    switch (flag) {
        case eDma_Flags_TC: {
            g_dma_clear_flags_fp_lut[stream].clear_tc_flag_fp(g_dma_desc_lut[stream].dma);
        } break;
        case eDma_Flags_HT: {
            g_dma_clear_flags_fp_lut[stream].clear_ht_flag_fp(g_dma_desc_lut[stream].dma);
        } break;
        case eDma_Flags_TE: {
            g_dma_clear_flags_fp_lut[stream].clear_te_flag_fp(g_dma_desc_lut[stream].dma);
        } break;
        default: {
            return false;
        }
    }

    return true;
}

bool DMA_Driver_ClearAllFlags (const eDma_t stream) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    if (g_dma_is_active_flags_fp_lut[stream].is_active_tc_flag_fp(g_dma_desc_lut[stream].dma)) {
        g_dma_clear_flags_fp_lut[stream].clear_tc_flag_fp(g_dma_desc_lut[stream].dma);
    }

    if (g_dma_is_active_flags_fp_lut[stream].is_active_ht_flag_fp(g_dma_desc_lut[stream].dma)) {
        g_dma_clear_flags_fp_lut[stream].clear_ht_flag_fp(g_dma_desc_lut[stream].dma);
    }

    if (g_dma_is_active_flags_fp_lut[stream].is_active_te_flag_fp(g_dma_desc_lut[stream].dma)) {
        g_dma_clear_flags_fp_lut[stream].clear_te_flag_fp(g_dma_desc_lut[stream].dma);
    }

    return true;
}

bool DMA_Driver_EnableIt (const eDma_t stream, const eDma_Flags_t flag) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!DMA_Driver_IsCorrectFlag(flag)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    switch (flag) {
        case eDma_Flags_TC: {
            LL_DMA_EnableIT_TC(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
        } break;
        case eDma_Flags_HT: {
            LL_DMA_EnableIT_HT(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
        } break;
        case eDma_Flags_TE: {
            LL_DMA_EnableIT_TE(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
        } break;
        default: {
            return false;
        }
    }

    return true;
}

bool DMA_Driver_EnableItAll (const eDma_t stream) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    LL_DMA_EnableIT_TC(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
    LL_DMA_EnableIT_HT(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
    LL_DMA_EnableIT_TE(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);

    return true;
}

bool DMA_Driver_DisableIt (const eDma_t stream, const eDma_Flags_t flag) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!DMA_Driver_IsCorrectFlag(flag)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    switch (flag) {
        case eDma_Flags_TC: {
            LL_DMA_DisableIT_TC(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
        } break;
        case eDma_Flags_HT: {
            LL_DMA_DisableIT_HT(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
        } break;
        case eDma_Flags_TE: {
            LL_DMA_DisableIT_TE(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
        } break;
        default: {
            return false;
        }
    }

    return true;
}

bool DMA_Driver_DisableItAll (const eDma_t stream) {
    if (!DMA_Config_IsCorrectDma(stream)) {
        return false;
    }

    if (!g_dynamic_dma_lut[stream].is_init) {
        return false;
    }

    LL_DMA_DisableIT_TC(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
    LL_DMA_DisableIT_HT(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);
    LL_DMA_DisableIT_TE(g_dma_desc_lut[stream].dma, g_dma_desc_lut[stream].stream);

    return true;
}

#endif /* ENABLE_DMA */
