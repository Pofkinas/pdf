/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "gpio_config.h"

#include <stdbool.h>
#include "gpio_driver.h"

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
const static sGpioDesc_t g_static_gpio_lut[eGpio_Last] = {
    [eGpio_DebugTx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_2,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpio_DebugRx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_3,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpio_OnboardLed] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_PulseLed] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_StartButton] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_uRosTx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_9,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpio_MotorA_A1] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_MotorA_A2] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_MotorB_A1] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_MotorB_A2] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_Tcrt5000_Right] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_Tcrt5000_Left] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_6,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_Ws2812B_1] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_Ws2812B_2] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_I2c1_SCL] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_8,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_OPENDRAIN,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_4
    },
    [eGpio_I2c1_SDA] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_9,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_OPENDRAIN,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_4
    },
    [eGpio_vl53l0_Xshut_1] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_vl53l0_Xshut_2] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_all_defined = false;

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

void GPIO_Config_DefineAll(void) {
    if (g_is_all_defined) {
        return;
    }

    for (eGpio_t gpio = eGpio_First; gpio < eGpio_Last; gpio++) {
        GPIO_Driver_DefinePin(g_static_gpio_lut[gpio]);
    }

    g_is_all_defined = true;

    return;
}
