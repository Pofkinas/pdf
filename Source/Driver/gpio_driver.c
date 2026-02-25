/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "gpio_driver.h"

#if defined(ENABLE_GPIO)

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

static sGpioDesc_t g_gpio_lut[eGpio_Last] = {0};
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

    LL_GPIO_InitTypeDef gpio_init_struct = {0};

    g_is_all_pin_initialized = true;

    for (eGpio_t pin = eGpio_First; pin < eGpio_Last; pin++) {
        const sGpioDesc_t *desc = GPIO_Config_GetGpioDesc(pin);
        
        if (NULL == desc) {
            g_is_all_pin_initialized = false;
            return false;
        }
        
        g_gpio_lut[pin] = *desc;

        LL_AHB1_GRP1_EnableClock(g_gpio_lut[pin].clock);
        LL_GPIO_ResetOutputPin(g_gpio_lut[pin].port, g_gpio_lut[pin].pin);
        
        gpio_init_struct.Pin = g_gpio_lut[pin].pin;
        gpio_init_struct.Mode = g_gpio_lut[pin].mode;
        gpio_init_struct.Speed = g_gpio_lut[pin].speed;
        gpio_init_struct.OutputType = g_gpio_lut[pin].output;
        gpio_init_struct.Pull = g_gpio_lut[pin].pull;
        gpio_init_struct.Alternate = g_gpio_lut[pin].alternate;

        if (ERROR == LL_GPIO_Init(g_gpio_lut[pin].port, &gpio_init_struct)) {
            g_is_all_pin_initialized = false;
        }
    }

    return g_is_all_pin_initialized;
}

bool GPIO_Driver_WritePin (const eGpio_t gpio_pin, const bool pin_state) {
    if (!g_is_all_pin_initialized) {
        return false;
    }
    
    if (!GPIO_Config_IsCorrectGpio(gpio_pin)) {
        return false;
    }

    if (LL_GPIO_MODE_OUTPUT != LL_GPIO_GetPinMode(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin)) {
        return false;
    }

    if (pin_state) {
        LL_GPIO_SetOutputPin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);
    } else {
        LL_GPIO_ResetOutputPin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);
    }

    return true;
}

bool GPIO_Driver_ReadPin (const eGpio_t gpio_pin, bool *pin_state) {
    if (!g_is_all_pin_initialized) {
        return false;
    }
    
    if (!GPIO_Config_IsCorrectGpio(gpio_pin)) {
        return false;
    }

    if (NULL == pin_state) {
        return false;
    }

    switch (g_gpio_lut[gpio_pin].mode) {
        case LL_GPIO_MODE_INPUT: {
            *pin_state = (0 != (LL_GPIO_ReadInputPort(g_gpio_lut[gpio_pin].port) & g_gpio_lut[gpio_pin].pin));
        } break;
        case LL_GPIO_MODE_OUTPUT: {
            *pin_state = (0 != (LL_GPIO_ReadOutputPort(g_gpio_lut[gpio_pin].port) & g_gpio_lut[gpio_pin].pin));
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

bool GPIO_Driver_TogglePin (const eGpio_t gpio_pin) {
    if (! g_is_all_pin_initialized) {
        return false;
    }

    if (!GPIO_Config_IsCorrectGpio(gpio_pin)) {
        return false;
    }

    LL_GPIO_TogglePin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);

    return true;
}

bool GPIO_Driver_ResetPin (const eGpio_t gpio_pin) {
    if (!g_is_all_pin_initialized) {
        return false;
    }
    
    if (!GPIO_Config_IsCorrectGpio(gpio_pin)) {
        return false;
    }

    LL_GPIO_SetPinMode(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin);
    LL_GPIO_SetPinMode(g_gpio_lut[gpio_pin].port, g_gpio_lut[gpio_pin].pin, g_gpio_lut[gpio_pin].mode);

    return true;
}

#endif /* ENABLE_GPIO */
