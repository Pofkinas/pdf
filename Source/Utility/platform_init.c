/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "platform_init.h"

#include "ws2812b_driver.h"
#include "uart_driver.h"
#include "pwm_driver.h"
#include "timer_driver.h"
#include "motor_driver.h"
#include "i2c_driver.h"
#include "gpio_driver.h"
#include "exti_driver.h"
#include "dma_driver.h"

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
#ifdef USE_WS2812B
const static sWs2812bStaticDesc_t g_static_ws2812b_lut[eWs2812bDriver_Last] = {
    #ifdef USE_WS2812B_1
    [eWs2812bDriver_1] = {
        .timer = eTimerDriver_TIM5,
        .pwm_device = ePwmDevice_Ws2812b_1,
        .dma_stream = eDmaDriver_Ws2812b_1,
        .total_led = WS2812B_1_LED_COUNT
    },
    #define USE_DMA
    #endif

    #ifdef USE_WS2812B_2
    [eWs2812bDriver_2] = {
        .timer = eTimerDriver_TIM5,
        .pwm_device = ePwmDevice_Ws2812b_2,
        .dma_stream = eDmaDriver_Ws2812b_2,
        .total_led = WS2812B_2_LED_COUNT
    },
    #define USE_DMA
    #endif
};
#endif

#ifdef USE_UART
const static sUartDesc_t g_static_uart_lut[eUartDriver_Last] = {
    #ifdef USE_UART_DEBUG
    [eUartDriver_Debug] = {
        .periph = USART2,
        .baud = 115200,
        .data_bits = LL_USART_DATAWIDTH_8B,
        .stop_bits = LL_USART_STOPBITS_1,
        .parity = LL_USART_PARITY_NONE,
        .direction = LL_USART_DIRECTION_TX_RX,
        .flow_control = LL_USART_HWCONTROL_NONE,
        .oversample = LL_USART_OVERSAMPLING_16,
        .clock = LL_APB1_GRP1_PERIPH_USART2,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .nvic = USART2_IRQn,
        .ring_buffer_capacity = UART_DEBUG_BUFFER_CAPACITY
    }
    #endif

    #ifdef USE_UART_UROS_TX
    [eUartDriver_uRos] = {
        .periph = USART1,
        .baud = 115200,
        .data_bits = LL_USART_DATAWIDTH_8B,
        .stop_bits = LL_USART_STOPBITS_1,
        .parity = LL_USART_PARITY_NONE,
        .direction = LL_USART_DIRECTION_TX,
        .flow_control = LL_USART_HWCONTROL_NONE,
        .oversample = LL_USART_OVERSAMPLING_16,
        .clock = LL_APB2_GRP1_PERIPH_USART1,
        .enable_clock_fp = LL_APB2_GRP1_EnableClock,
        .nvic = USART1_IRQn,
    }
    #endif
};
#endif

#ifdef USE_PWM
const static sPwmOcChannelDesc_t g_static_pwm_lut[ePwmDevice_Last] = {
    #ifdef USE_PULSE_LED
    [ePwmDevice_PulseLed] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH1,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH1,
        .is_dma_request_enabled = false
    },
    #endif
    
    #ifdef USE_MOTOR_A
    [ePwmDevice_MotorA_A1] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH1,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH1,
        .is_dma_request_enabled = false,
    },
    [ePwmDevice_MotorA_A2] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH2,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH2,
        .is_dma_request_enabled = false,
    },
    #endif

    #ifdef USE_MOTOR_B
    [ePwmDevice_MotorB_A1] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH3,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH3,
        .is_dma_request_enabled = false,
    },
    [ePwmDevice_MotorB_A2] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH4,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH4,
        .is_dma_request_enabled = false,
    },
    #endif

    #ifdef USE_WS2812B_1
    [ePwmDevice_Ws2812b_1] = {
        .periph = TIM5,
        .timer = eTimerDriver_TIM5,
        .channel = LL_TIM_CHANNEL_CH2,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_ENABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH2,
        .is_dma_request_enabled = true,
        .dma_request_fp = LL_TIM_EnableDMAReq_CC2,
        .get_ccr_fp = LL_TIM_OC_GetCompareCH2
    },
    #endif

    #ifdef USE_WS2812B_2
    [ePwmDevice_Ws2812b_2] = {
        .periph = TIM5,
        .timer = eTimerDriver_TIM5,
        .gpio_pin = eGpioPin_Ws2812B_2,
        .channel = LL_TIM_CHANNEL_CH1,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_ENABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH1,
        .is_dma_request_enabled = true,
        .dma_request_fp = LL_TIM_EnableDMAReq_CC1,
        .get_ccr_fp = LL_TIM_OC_GetCompareCH1
    }
    #endif
};
#endif

#ifdef USE_MOTOR
const static sMotorConstDesc_t g_static_motor_lut[eMotorDriver_Last] = {
    #ifdef USE_MOTOR_A
    [eMotorDriver_A] = {
        .timer = eTimerDriver_TIM3,
        .fwd_channel = ePwmDevice_MotorA_A2,
        .rev_channel = ePwmDevice_MotorA_A1
    },
    #endif

    #ifdef USE_MOTOR_B
    [eMotorDriver_B] = {
        .timer = eTimerDriver_TIM3,
        .fwd_channel = ePwmDevice_MotorB_A2,
        .rev_channel = ePwmDevice_MotorB_A1
    }
    #endif
};
#endif

#ifdef USE_I2C
static const sI2cDesc_t g_static_i2c_lut[eI2cDriver_Last] = {
    #ifdef USE_I2C1
    [eI2cDriver_1] = {
        .periph = I2C1,
        .peripheral_mode = LL_I2C_MODE_I2C,
        .clock_speed = 100000,
        .duty_cycle = LL_I2C_DUTYCYCLE_2,
        .own_address1 = 0,
        .own_address2 = 0,
        .type_acknowledge = LL_I2C_ACK,
        .own_addr_size = LL_I2C_OWNADDRESS1_7BIT,
        .clock = LL_APB1_GRP1_PERIPH_I2C1,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .is_enabled_it = true,
        .nvic = I2C1_EV_IRQn,
        .scl_pin = eGpioPin_I2c1_SCL,
        .sda_pin = eGpioPin_I2c1_SDA
    }
    #endif
};
#endif

#ifdef USE_EXTI
const static sExtiDesc_t g_static_exti_lut[eExtiDriver_Last] = {
    #ifdef USE_START_BUTTON
    [eExtiDriver_StartButton] = {
        .pin = eGpioPin_StartButton,
        .system_port = LL_SYSCFG_EXTI_PORTC,
        .system_line = LL_SYSCFG_EXTI_LINE0,
        .line_0_31 = LL_EXTI_LINE_0,
        .command = ENABLE,
        .mode = LL_EXTI_MODE_IT,
        .trigger = LL_EXTI_TRIGGER_FALLING,
        .nvic = EXTI0_IRQn,
    },
    #endif

    #ifdef USE_TCRT5000_RIGHT
    [eExtiDriver_Tcrt5000_Right] = {
        .pin = eGpioPin_Tcrt5000_Right,
        .system_port = LL_SYSCFG_EXTI_PORTC,
        .system_line = LL_SYSCFG_EXTI_LINE1,
        .line_0_31 = LL_EXTI_LINE_1,
        .command = ENABLE,
        .mode = LL_EXTI_MODE_IT,
        .trigger = LL_EXTI_TRIGGER_RISING_FALLING,
        .nvic = EXTI1_IRQn,
    },
    #endif

    #ifdef USE_TCRT5000_LEFT
    [eExtiDriver_Tcrt5000_Left] = {
        .pin = eGpioPin_Tcrt5000_Left,
        .system_port = LL_SYSCFG_EXTI_PORTA,
        .system_line = LL_SYSCFG_EXTI_LINE6,
        .line_0_31 = LL_EXTI_LINE_6,
        .command = ENABLE,
        .mode = LL_EXTI_MODE_IT,
        .trigger = LL_EXTI_TRIGGER_RISING_FALLING,
        .nvic = EXTI9_5_IRQn,
    },
    #endif
};
#endif

#ifdef USE_DMA
const static sDmaStaticDesc_t g_static_dma_desc_lut[eDmaDriver_Last] = {
    #ifdef USE_WS2812B_1
    [eDmaDriver_Ws2812b_1] = {
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
    },
    #endif

    #ifdef USE_WS2812B_2
    [eDmaDriver_Ws2812b_2] = {
        .dma = DMA1,
        .enable_clock_fp = LL_AHB1_GRP1_EnableClock,
        .clock = LL_AHB1_GRP1_PERIPH_DMA1,
        .nvic = DMA1_Stream2_IRQn,
        .channel = LL_DMA_CHANNEL_6,
        .stream = LL_DMA_STREAM_2,
        .data_direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
        .mode = LL_DMA_MODE_CIRCULAR,
        .periph_or_src_increment_mode = LL_DMA_PERIPH_NOINCREMENT,
        .mem_or_dest_increment_mode = LL_DMA_MEMORY_INCREMENT,
        .periph_or_src_size = LL_DMA_PDATAALIGN_WORD,
        .mem_or_dest_size = LL_DMA_MDATAALIGN_WORD,
        .priority_level = LL_DMA_PRIORITY_MEDIUM,
        .fifo_mode_fp = LL_DMA_DisableFifoMode,
    },
    #endif
};

static sDmaIsActiveFlags_t g_dma_is_active_flags_fp_lut[eDmaDriver_Last] = {
    #ifdef USE_WS2812B_1
    [eDmaDriver_Ws2812b_1] = {
        .is_active_tc_flag_fp = LL_DMA_IsActiveFlag_TC4,
        .is_active_ht_flag_fp = LL_DMA_IsActiveFlag_HT4,
        .is_active_te_flag_fp = LL_DMA_IsActiveFlag_TE4
    },
    #endif

    #ifdef USE_WS2812B_2
    [eDmaDriver_Ws2812b_2] = {
        .is_active_tc_flag_fp = LL_DMA_IsActiveFlag_TC2,
        .is_active_ht_flag_fp = LL_DMA_IsActiveFlag_HT2,
        .is_active_te_flag_fp = LL_DMA_IsActiveFlag_TE2
    },
    #endif
};

const static sDmaClearFlags_t g_dma_clear_flags_fp_lut[eDmaDriver_Last] = {
    #ifdef USE_WS2812B_1
    [eDmaDriver_Ws2812b_1] = {
        .clear_tc_flag_fp = LL_DMA_ClearFlag_TC4,
        .clear_ht_flag_fp = LL_DMA_ClearFlag_HT4,
        .clear_te_flag_fp = LL_DMA_ClearFlag_TE4
    },
    #endif

    #ifdef USE_WS2812B_2
    [eDmaDriver_Ws2812b_2] = {
        .clear_tc_flag_fp = LL_DMA_ClearFlag_TC2,
        .clear_ht_flag_fp = LL_DMA_ClearFlag_HT2,
        .clear_te_flag_fp = LL_DMA_ClearFlag_TE2
    },
    #endif
};
#endif

const static sTimerDesc_t g_static_timer_lut[eTimerDriver_Last] = {
    [eTimerDriver_TIM10] = {
        .periph = TIM10,
        .prescaler = 999,
        .counter_mode = LL_TIM_COUNTERMODE_UP,
        .auto_reload = 65535,
        .clock_division = LL_TIM_CLOCKDIVISION_DIV1,
        .clock = LL_APB2_GRP1_PERIPH_TIM10,
        .clock_source_fp = NULL,
        .enable_clock_fp = LL_APB2_GRP1_EnableClock,
        .nvic = TIM1_UP_TIM10_IRQn,
        .enable_interupt = true,
        .auto_relead_preload_fp = LL_TIM_DisableARRPreload,
        .master_slave_mode_fp = NULL
    },
    
    #if defined(USE_MOTOR_A) || defined(USE_MOTOR_B) || defined(USE_PULSE_LED)
    [eTimerDriver_TIM3] = {
        .periph = TIM3,
        .prescaler = 0,
        .counter_mode = LL_TIM_COUNTERMODE_UP,
        .auto_reload = 256,
        .clock_division = LL_TIM_CLOCKDIVISION_DIV1,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .clock = LL_APB1_GRP1_PERIPH_TIM3,
        .clock_source_fp = LL_TIM_SetClockSource,
        .clock_source = LL_TIM_CLOCKSOURCE_INTERNAL,
        .enable_interupt = false,
        .auto_relead_preload_fp = LL_TIM_DisableARRPreload,
        .master_slave_mode_fp = LL_TIM_DisableMasterSlaveMode,
        .set_trigger = LL_TIM_SetTriggerOutput,
        .triger_sync = LL_TIM_TRGO_RESET
    },
    #endif

    #if defined(USE_WS2812B_1) || defined(USE_WS2812B_2)
    [eTimerDriver_TIM5] = {
        .periph = TIM5,
        .prescaler = 0,
        .counter_mode = LL_TIM_COUNTERMODE_UP,
        .auto_reload = 125,
        .clock_division = LL_TIM_CLOCKDIVISION_DIV1,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .clock = LL_APB1_GRP1_PERIPH_TIM5,
        .clock_source_fp = LL_TIM_SetClockSource,
        .clock_source = LL_TIM_CLOCKSOURCE_INTERNAL,
        .enable_interupt = false,
        .auto_relead_preload_fp = LL_TIM_EnableARRPreload,
        .master_slave_mode_fp = LL_TIM_DisableMasterSlaveMode,
        .set_trigger = LL_TIM_SetTriggerOutput,
        .triger_sync = LL_TIM_TRGO_RESET
    },
    #endif
};

const static sGpioDesc_t g_static_gpio_lut[eGpioPin_Last] = {
    #ifdef USE_UART_DEBUG
    [eGpioPin_DebugTx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_2,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpioPin_DebugRx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_3,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    #endif

    #ifdef USE_ONBOARD_LED
    [eGpioPin_OnboardLed] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    #endif

    #ifdef USE_PULSE_LED
    [eGpioPin_PulseLed] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    #endif

    #ifdef USE_START_BUTTON
    [eGpioPin_StartButton] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    #endif

    #ifdef USE_UART_UROS_TX
    [eGpioPin_uRosTx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_9,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    #endif

    #ifdef USE_MOTOR_A
    [eGpioPin_MotorA_A1] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpioPin_MotorA_A2] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    #endif

    #ifdef USE_MOTOR_B
    [eGpioPin_MotorB_A1] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpioPin_MotorB_A2] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    #endif

    #ifdef USE_TCRT5000_RIGHT
    [eGpioPin_Tcrt5000_Right] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    #endif

    #ifdef USE_TCRT5000_LEFT
    [eGpioPin_Tcrt5000_Left] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_6,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    #endif

    #ifdef USE_WS2812B_1
    [eGpioPin_Ws2812B_1] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_2
    },
    #endif

    #ifdef USE_WS2812B_2
    [eGpioPin_Ws2812B_2] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_2
    },
    #endif

    #ifdef USE_I2C1
    [eGpioPin_I2c1_SCL] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_8,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_OPENDRAIN,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_4
    },
    [eGpioPin_I2c1_SDA] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_9,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_OPENDRAIN,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_4
    },
    #endif

    #ifdef USE_VL53L0_XSHUT1
    [eGpioPin_vl53l0_Xshut_1] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    #endif

    #ifdef USE_VL53L0_XSHUT2
    [eGpioPin_vl53l0_Xshut_2] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    #endif
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_initialized = false;

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

void Platform_Init (void) {
    if (g_is_initialized) {
        return;
    }
    
    #ifdef USE_VL53L0X
    WS2812B_Driver_DefinePherips(g_static_ws2812b_lut);
    #endif

    #ifdef USE_UART
    UART_Driver_DefinePerips(g_static_uart_lut);
    #endif

    #ifdef USE_PWM
    PWM_Driver_DefinePerips(g_static_pwm_lut);
    #endif

    #ifdef USE_MOTOR
    Motor_Driver_DefinePerips(g_static_motor_lut);
    #endif

    #ifdef USE_I2C
    I2C_Driver_DefinePerips(g_static_i2c_lut);
    #endif

    #ifdef USE_EXTI
    Exti_Driver_DefinePerips(g_static_exti_lut);
    #endif

    #ifdef USE_DMA
    DMA_Driver_DefinePerips(g_static_dma_desc_lut, g_dma_is_active_flags_fp_lut, g_dma_clear_flags_fp_lut);
    #endif

    Timer_Driver_DefineTimers(g_static_timer_lut);
    GPIO_Driver_DefinePins(g_static_gpio_lut);

    g_is_initialized = true;

    return;
}
