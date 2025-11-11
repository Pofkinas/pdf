#ifndef CONFIG_GPIO_CONFIG_H_
#define CONFIG_GPIO_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "platform_config.h"

#include GPIO_DRIVER
#include BUS_DRIVER

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eGpio {
    eGpio_First = 0,
    eGpio_DebugTx = eGpio_First,
    eGpio_DebugRx,
    eGpio_PulseLed,
    eGpio_OnboardLed,
    eGpio_StartButton,
    eGpio_MotorA_A1,
    eGpio_MotorA_A2,
    eGpio_Ws2812B_1,
    eGpio_I2c1_SCL,
    eGpio_I2c1_SDA,
    eGpio_Vl53l0_Xshut_1,
    eGpio_Last
} eGpio_t;

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
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool GPIO_Config_IsCorrectGpio (const eGpio_t gpio);
const sGpioDesc_t *GPIO_Config_GetGpioDesc (const eGpio_t gpio_pin);

#endif /* CONFIG_GPIO_CONFIG_H_ */
