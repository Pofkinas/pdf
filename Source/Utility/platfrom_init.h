#ifndef SOURCE_UTILITY_PLATFORM_INIT_H_
#define SOURCE_UTILITY_PLATFORM_INIT_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_dma.h"
#include "timer_driver.h"
#include "pwm_driver.h"
#include "gpio_driver.h"
#include "dma_driver.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef struct sWs2812bStaticDesc {
    eTimerDriver_t timer;
    ePwmDevice_t pwm_device;
    eDmaDriver_t dma_stream;
    size_t total_led;
} sWs2812bStaticDesc_t;

typedef struct sUartDesc {
    USART_TypeDef *periph;
    uint32_t baud;
    uint32_t data_bits;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t direction;
    uint32_t flow_control;
    uint32_t oversample;
    uint32_t clock;
    void (*enable_clock_fp) (uint32_t);
    IRQn_Type nvic;
    size_t ring_buffer_capacity;
} sUartDesc_t;

typedef struct sTimerDesc {
    TIM_TypeDef *periph;
    uint16_t prescaler;
    uint32_t counter_mode;
    uint32_t auto_reload;
    uint32_t clock_division;
    void (*enable_clock_fp) (uint32_t);
    uint32_t clock;
    void (*clock_source_fp) (TIM_TypeDef *, uint32_t);
    uint32_t clock_source;
    bool enable_interupt;
    IRQn_Type nvic;
    void (*auto_relead_preload_fp) (TIM_TypeDef *);
    void (*master_slave_mode_fp) (TIM_TypeDef *);
    void (*set_slave_mode_fp) (TIM_TypeDef *, uint32_t);
    uint32_t slave_mode;
    void (*set_trigger) (TIM_TypeDef *, uint32_t);
    uint32_t triger_sync;
} sTimerDesc_t;

typedef struct sPwmOcChannelDesc {
    TIM_TypeDef *periph;
    eTimerDriver_t timer;
    uint32_t channel;
    uint32_t mode;
    uint32_t oc_state;
    uint32_t ocn_state;
    uint32_t compare_value;
    uint32_t oc_polarity;
    uint32_t ocn_polarity;
    uint32_t oc_idle;
    uint32_t ocn_idle;
    void (*fast_mode_fp) (TIM_TypeDef *, uint32_t);
    void (*compare_preload_fp) (TIM_TypeDef *, uint32_t);
    void (*compare_value_fp) (TIM_TypeDef *, uint32_t);
    bool is_dma_request_enabled;
    void (*dma_request_fp) (TIM_TypeDef *);
    uint32_t (*get_ccr_fp) (const TIM_TypeDef *);
} sPwmOcChannelDesc_t;

typedef struct sMotorConstDesc {
    eTimerDriver_t timer;
    ePwmDevice_t fwd_channel;
    ePwmDevice_t rev_channel;
} sMotorConstDesc_t;

typedef struct sI2cDesc {
    I2C_TypeDef *periph;
    uint32_t peripheral_mode;
    uint32_t clock_speed;
    uint32_t duty_cycle;
    uint32_t analog_filter;
    uint32_t digital_filter;
    uint32_t own_address1;
    uint32_t own_address2;
    uint32_t type_acknowledge;
    uint32_t own_addr_size;
    uint32_t clock;
    void (*enable_clock_fp) (uint32_t);
    bool is_enabled_it;
    IRQn_Type nvic;
    eGpioPin_t scl_pin;
    eGpioPin_t sda_pin;
} sI2cDesc_t;

typedef struct sGpioDesc {
    GPIO_TypeDef *port;
    uint32_t pin;
    uint32_t mode;
    uint32_t speed;
    uint32_t pull;
    uint32_t output;
    uint32_t clock;
    uint32_t alternate;
} sGpioDesc_t;

typedef struct sExtiDesc {
    eGpioPin_t pin;
    uint32_t system_port;
    uint32_t system_line;
    uint32_t line_0_31;
    FunctionalState command;
    uint8_t mode;
    uint8_t trigger;
    IRQn_Type nvic;
} sExtiDesc_t;

typedef struct sDmaStaticDesc {
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
} sDmaStaticDesc_t;

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

void Platform_Init (void);

#endif /* SOURCE_UTILITY_PLATFORM_INIT_H_ */
