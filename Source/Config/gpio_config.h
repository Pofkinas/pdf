#ifndef CONFIG_GPIO_CONFIG_H_
#define CONFIG_GPIO_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdint.h>
#include "platform_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eGpio {
    eGpio_First = 0,
    eGpio_DebugTx = eGpio_First,
    eGpio_DebugRx,
    eGpio_OnboardLed,
    eGpio_PulseLed,
    eGpio_StartButton,
    eGpio_uRosTx,
    eGpio_MotorA_A1,
    eGpio_MotorA_A2,
    eGpio_MotorB_A1,
    eGpio_MotorB_A2,
    eGpio_Tcrt5000_Right,
    eGpio_Tcrt5000_Left,
    eGpio_Ws2812B_1,
    eGpio_Ws2812B_2,
    eGpio_I2c1_SCL,
    eGpio_I2c1_SDA,
    eGpio_vl53l0_Xshut_1,
    eGpio_vl53l0_Xshut_2,
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
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void GPIO_Config_DefineAll(void);

#endif /* CONFIG_GPIO_CONFIG_H_ */
