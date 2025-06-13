/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "gpio_driver.h"

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

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

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
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

static bool g_is_all_pin_initialized = false;

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

bool GPIO_Driver_InitAllPins (void) {
    if (g_is_all_pin_initialized) {
        return true;
    }

    if (eGpioPin_Last == 1) {
        return false;
    }

    LL_GPIO_InitTypeDef gpio_init_struct = {0};

    g_is_all_pin_initialized = true;

    for (eGpioPin_t pin = (eGpioPin_First + 1); pin < eGpioPin_Last; pin++) {
        LL_AHB1_GRP1_EnableClock(g_static_gpio_lut[pin].clock);
        LL_GPIO_ResetOutputPin(g_static_gpio_lut[pin].port, g_static_gpio_lut[pin].pin);
        
        gpio_init_struct.Pin = g_static_gpio_lut[pin].pin;
        gpio_init_struct.Mode = g_static_gpio_lut[pin].mode;
        gpio_init_struct.Speed = g_static_gpio_lut[pin].speed;
        gpio_init_struct.OutputType = g_static_gpio_lut[pin].output;
        gpio_init_struct.Pull = g_static_gpio_lut[pin].pull;
        gpio_init_struct.Alternate = g_static_gpio_lut[pin].alternate;

        if (LL_GPIO_Init(g_static_gpio_lut[pin].port, &gpio_init_struct) == ERROR) {
            g_is_all_pin_initialized = false;
        }
    }

    return g_is_all_pin_initialized;
}

bool GPIO_Driver_WritePin (const eGpioPin_t gpio_pin, const bool pin_state) {
    if ((gpio_pin <= eGpioPin_First) || (gpio_pin >= eGpioPin_Last)) {
        return false;
    }

    if (LL_GPIO_GetPinMode(g_static_gpio_lut[gpio_pin].port, g_static_gpio_lut[gpio_pin].pin) != LL_GPIO_MODE_OUTPUT) {
        return false;
    }

    if (pin_state) {
        LL_GPIO_SetOutputPin(g_static_gpio_lut[gpio_pin].port, g_static_gpio_lut[gpio_pin].pin);
    } else {
        LL_GPIO_ResetOutputPin(g_static_gpio_lut[gpio_pin].port, g_static_gpio_lut[gpio_pin].pin);
    }

    return true;
}

bool GPIO_Driver_ReadPin (const eGpioPin_t gpio_pin, bool *pin_state) {
    if ((gpio_pin <= eGpioPin_First) || (gpio_pin >= eGpioPin_Last)) {
        return false;
    }

    if (pin_state == NULL) {
        return false;
    }

    switch (g_static_gpio_lut[gpio_pin].mode) {
        case LL_GPIO_MODE_INPUT: {
            *pin_state = (LL_GPIO_ReadInputPort(g_static_gpio_lut[gpio_pin].port) & g_static_gpio_lut[gpio_pin].pin) != 0;
        } break;
        case LL_GPIO_MODE_OUTPUT: {
            *pin_state = (LL_GPIO_ReadOutputPort(g_static_gpio_lut[gpio_pin].port) & g_static_gpio_lut[gpio_pin].pin) != 0;
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

bool GPIO_Driver_TogglePin (const eGpioPin_t gpio_pin) {
    if ((gpio_pin <= eGpioPin_First) || (gpio_pin >= eGpioPin_Last)) {
        return false;
    }

    LL_GPIO_TogglePin(g_static_gpio_lut[gpio_pin].port, g_static_gpio_lut[gpio_pin].pin);

    return true;
}

bool GPIO_Driver_ResetPin (const eGpioPin_t gpio_pin) {
    if ((gpio_pin <= eGpioPin_First) || (gpio_pin >= eGpioPin_Last)) {
        return false;
    }

    LL_GPIO_SetPinMode(g_static_gpio_lut[gpio_pin].port, g_static_gpio_lut[gpio_pin].pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(g_static_gpio_lut[gpio_pin].port, g_static_gpio_lut[gpio_pin].pin);
    LL_GPIO_SetPinMode(g_static_gpio_lut[gpio_pin].port, g_static_gpio_lut[gpio_pin].pin, g_static_gpio_lut[gpio_pin].mode);

    return true;
}
