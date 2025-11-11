#ifndef CONFIG_WS2812B_CONFIG_H_
#define CONFIG_WS2812B_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "cmsis_os2.h"
#include "timer_config.h"
#include "pwm_config.h"
#include "dma_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define WS2812B_1_LED_COUNT 16

#define LED_DATA_CHANNELS 3
#define LED_RESOLUTION 2
#define LATCH_LED_TRANSFERS 2
#define SINGLE_DATA_TRANSFER_TIME_NS 1250

#define MUTEX_TIMEOUT 0U
#define REFRESH_RATE 33U // 30 FPS
#define DEFAULT_FLAG_TIMEOUT 50U

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eWs2812b {
    eWs2812b_First = 0,
    eWs2812b_1 = eWs2812b_First,
    eWs2812b_Last
} eWs2812b_t;

typedef struct sWs2812bDesc {
    eTimer_t timer;
    ePwm_t pwm_device;
    eDma_t dma_stream;
    size_t total_led;
} sWs2812bDesc_t;

typedef struct sWs2812bControlDesc {
    eWs2812b_t device;
    size_t max_led;
    osTimerAttr_t timer_attributes;
    osMutexAttr_t mutex_attributes;
    osEventFlagsAttr_t flag_attributes;
} sWs2812bControlDesc_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool WS2812B_Config_IsCorrectWs2812b (const eWs2812b_t ws2812b_device);
const sWs2812bDesc_t *WS2812B_Config_GetWs2812bDesc (const eWs2812b_t ws2812b_device);
const sWs2812bControlDesc_t *WS2812B_Config_GetWs2812bControlDesc (const eWs2812b_t ws2812b_device);

#endif /* CONFIG_WS2812B_CONFIG_H_ */
