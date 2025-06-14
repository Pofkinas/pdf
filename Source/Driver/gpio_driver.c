/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

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

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sGpioDesc_t g_gpio_lut[eGpioPin_Last];
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

void GPIO_Driver_DefinePins (const sGpioDesc_t *gpio_lut) {
    if (gpio_lut == NULL) {
        return;
    }

    g_gpio_lut = gpio_lut;

    g_is_all_pin_initialized = false;

    return;
}

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
        LL_AHB1_GRP1_EnableClock(g_gpio_lut[pin].clock);
        LL_GPIO_ResetOutputPin(g_gpio_lut[pin].port, g_gpio_lut[pin].pin);
        
        gpio_init_struct.Pin = g_gpio_lut[pin].pin;
        gpio_init_struct.Mode = g_gpio_lut[pin].mode;
        gpio_init_struct.Speed = g_gpio_lut[pin].speed;
        gpio_init_struct.OutputType = g_gpio_lut[pin].output;
        gpio_init_struct.Pull = g_gpio_lut[pin].pull;
        gpio_init_struct.Alternate = g_gpio_lut[pin].alternate;

        if (LL_GPIO_Init(g_gpio_lut[pin].port, &gpio_init_struct) == ERROR) {
            g_is_all_pin_initialized = false;
        }
    }

    return g_is_all_pin_initialized;
}

bool GPIO_Driver_WritePin (const eGpioPin_t gpio_pin, const bool pin_state) {
    if ((gpio_pin <= eGpioPin_First) || (gpio_pin >= eGpioPin_Last)) {
        return false;
    }

    if (LL_GPIO_GetPinMode(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin) != LL_GPIO_MODE_OUTPUT) {
        return false;
    }

    if (pin_state) {
        LL_GPIO_SetOutputPin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);
    } else {
        LL_GPIO_ResetOutputPin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);
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

    switch (g_gpio_lut[gpio_pin].mode) {
        case LL_GPIO_MODE_INPUT: {
            *pin_state = (LL_GPIO_ReadInputPort(g_gpio_lut[gpio_pin].port) & g_gpio_lut[gpio_pin].pin) != 0;
        } break;
        case LL_GPIO_MODE_OUTPUT: {
            *pin_state = (LL_GPIO_ReadOutputPort(g_gpio_lut[gpio_pin].port) & g_gpio_lut[gpio_pin].pin) != 0;
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

    LL_GPIO_TogglePin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);

    return true;
}

bool GPIO_Driver_ResetPin (const eGpioPin_t gpio_pin) {
    if ((gpio_pin <= eGpioPin_First) || (gpio_pin >= eGpioPin_Last)) {
        return false;
    }

    LL_GPIO_SetPinMode(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);
    LL_GPIO_SetPinMode(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin, g_gpio_lut[gpio_pin].mode);

    return true;
}
