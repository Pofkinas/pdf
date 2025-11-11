/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include "cmsis_os.h"
#include "FreeRTOSConfig.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_usart.h"
#include "usart.h"
#include "cli_app.h"
#include "led_app.h"
#include "debug_api.h"
#include "timer_driver.h"
#include "baudrate.h"

#include "motor_app.h"
#include "io_api.h"
#include "i2c_api.h"
#include "vl53l0xv2_api.h"
#include "lcd_api.h"
#include "ws2812b_api.h"
#include "message.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_MAIN
CREATE_MODULE_NAME (MAIN)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_MAIN */

static const osThreadAttr_t g_test_thread_attributes = {
    .name = "TEST_APP_Thread",
    .stack_size = 128 * 16,
    .priority = (osPriority_t) osPriorityNormal
};

static const osEventFlagsAttr_t g_start_button_event_attributes = {
    .name = "Start_Button_Event",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static osEventFlagsId_t g_start_button_event = NULL;
static sMessage_t g_message = {.data = NULL, .size = 0};
static char buffer[32];

static sLedAnimationRainbow_t g_rainbow_animation = {
    .direction = eDirection_Up,
    .start_hsv_color = {.hue = 0, .saturation = 255, .value = 128},
    .segment_start_led = 2,
    .segment_end_led = 15,
    .speed = 1,
    .hue_step = 10,
    .frames_per_update = 1
};

static sLedAnimationDesc_t g_animation = {
    .device = eWs2812b_1,
    .animation = eLedAnimation_Rainbow,
    .brightness = 16,
    .data = &g_rainbow_animation
};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void SystemClock_Config (void);

volatile unsigned long ulHighFrequencyTimerTicks;

void configureTimerForRunTimeStats (void);
void TIM1_UP_TIM10_IRQnHandler (void);

unsigned long getRunTimeCounterValue (void);

void TEST_APP_TestThread (void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void SystemClock_Config (void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
    while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_3) {}
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_HSI_Enable();

    while(LL_RCC_HSI_IsReady() != 1) {}
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 100, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_Enable();

    while(LL_RCC_PLL_IsReady() != 1) {}
    while (LL_PWR_IsActiveFlag_VOS() == 0) {}
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}
    LL_SetSystemCoreClock(SYSTEM_CLOCK_HZ);

    if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK) {
        __disable_irq();
        while (1) {}
    }
}

void configureTimerForRunTimeStats (void) {
    ulHighFrequencyTimerTicks = 0;
    LL_TIM_EnableIT_UPDATE(TIM10);
    LL_TIM_EnableCounter(TIM10);
}

unsigned long getRunTimeCounterValue (void) {
    return ulHighFrequencyTimerTicks;
}

void TIM1_UP_TIM10_IRQHandler (void) {
    if (LL_TIM_IsActiveFlag_UPDATE(TIM10)) {
        ulHighFrequencyTimerTicks++;
        LL_TIM_ClearFlag_UPDATE(TIM10);
    }
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

int main (void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    SystemClock_Config();

    osKernelInitialize();

    // Init TIM10 for debbuging stack size
    Timer_Driver_InitAllTimers();
    Timer_Driver_Start(eTimer_TIM10);

    CLI_APP_Init(UART_2_BAUDRATE);
    LED_APP_Init();

    // Test motor app
    Motor_APP_Init();

    // Test IO
    g_start_button_event = osEventFlagsNew(&g_start_button_event_attributes);
    IO_API_Init(eIo_StartStopButton, g_start_button_event);

    // Test WS2812B
    WS2812B_API_InitAll();

    // Test thread
    osThreadNew(TEST_APP_TestThread, NULL, &g_test_thread_attributes);

    TRACE_INFO("Start OK\n");

    osKernelStart();

    while (1) {}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        HAL_IncTick();
    }
}

// #define SCAN_I2C

void TEST_APP_TestThread (void *arg) {
    // Test VL53L0X
    VL53L0X_API_InitAll();
    WS2812B_API_Reset(eWs2812b_1);

    // Test LCD
    LCD_API_InitAllLcd();

    bool is_button_pressed = false;
    uint16_t distance = 0;
    
    #ifdef SCAN_I2C
    for (uint8_t addr = 1; addr < 255; addr++) {
         if (I2C_API_Write(eI2c_1, addr, NULL, 0, 0, 0, 10)) {
             TRACE_INFO("Found I2C device at 0x%02X\n", addr);
         }

         osDelay(10);
    }
    #endif /* SCAN_I2C */

    while (1) {
        if (osEventFlagsWait(g_start_button_event, STARTSTOP_TRIGGERED_EVENT, osFlagsWaitAny, 50) == STARTSTOP_TRIGGERED_EVENT) {
            TRACE_INFO("Start/Stop button pressed!\n");

            is_button_pressed = !is_button_pressed;

            if (is_button_pressed) {
                WS2812B_API_AddAnimation(&g_animation);
                WS2812B_API_Start(eWs2812b_1);

                VL53L0X_API_StartMeasuring(eVl53l0x_1);
            } else {
                WS2812B_API_Stop(eWs2812b_1);
                WS2812B_API_ClearAnimations(eWs2812b_1);

                VL53L0X_API_StopMeasuring(eVl53l0x_1);

                LCD_API_Clear(eLcd_1);
            }
        }

        if (is_button_pressed) {
            if(VL53L0X_API_GetDistance(eVl53l0x_1, &distance, 200)) {
                g_message.size = sprintf(buffer, "%4d mm", distance);
                g_message.data = buffer;

                TRACE_INFO("Measured distance: %s\n", buffer);

                LCD_API_Print(eLcd_1, &g_message, eLcdRow_1, eLcdColumn_1, eLcdOption_Refresh);
            }
        }
    }

    osThreadYield();
}
