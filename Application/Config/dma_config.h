#ifndef CONFIG_DMA_CONFIG_H_
#define CONFIG_DMA_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "platform_config.h"

#include DMA_DRIVER
#include EXTI_DRIVER
#include BUS_DRIVER

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eDma{
    eDma_First = 0,
    eDma_Ws2812b_1 = eDma_First,
    eDma_Last
} eDma_t;

typedef struct sDmaDesc {
    DMA_TypeDef *dma;
    void (*enable_clock_fp) (uint32_t);
    uint32_t clock;
    IRQn_Type nvic;
    uint32_t channel;
    uint32_t stream;
    uint32_t data_direction;
    uint32_t mode;
    uint32_t periph_or_src_increment_mode;
    uint32_t mem_or_dest_increment_mode;
    uint32_t periph_or_src_size;
    uint32_t mem_or_dest_size;
    uint32_t priority_level;
    void (*fifo_mode_fp) (DMA_TypeDef*, uint32_t);
} sDmaDesc_t;

typedef struct sDmaIsActiveFlags {
    uint32_t (*is_active_tc_flag_fp) (DMA_TypeDef*);
    uint32_t (*is_active_ht_flag_fp) (DMA_TypeDef*);
    uint32_t (*is_active_te_flag_fp) (DMA_TypeDef*);
} sDmaIsActiveFlags_t;

typedef struct sDmaClearFlags {
    void (*clear_tc_flag_fp) (DMA_TypeDef*);
    void (*clear_ht_flag_fp) (DMA_TypeDef*);
    void (*clear_te_flag_fp) (DMA_TypeDef*);
} sDmaClearFlags_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool DMA_Config_IsCorrectDma (const eDma_t dma);
const sDmaDesc_t *DMA_Config_GetDmaDesc (const eDma_t dma);
const sDmaIsActiveFlags_t *DMA_Config_GetDmaIsActiveFlagsFp (const eDma_t dma);
const sDmaClearFlags_t *DMA_Config_GetDmaClearFlagsFp (const eDma_t dma);

#endif /* CONFIG_DMA_CONFIG_H_ */
