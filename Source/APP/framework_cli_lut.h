#ifndef SOURCE_APP_FRAMEWORK_CLI_LUT_H_
#define SOURCE_APP_FRAMEWORK_CLI_LUT_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cmd_api.h"
#include "framework_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

 /* clang-format off */
typedef enum eCliFrameworkCmd {
    eCliFrameworkCmd_First = 0,
    
    #ifdef USE_LED
    eCliFrameworkCmd_Led_Set,
    eCliFrameworkCmd_Led_Reset,
    eCliFrameworkCmd_Led_Toggle,
    eCliFrameworkCmd_Led_Blink,
    #endif
    
    #ifdef USE_PWM_LED
    eCliFrameworkCmd_Pwm_Led_SetBrightness,
    eCliFrameworkCmd_Pwm_Led_Pulse,
    #endif

    #ifdef ENABLE_MOTOR
    eCliDefaultCmd_Motors_Set,
    eCliDefaultCmd_Motors_Stop,
    #endif  /* ENABLE_MOTOR */
    
    eCliDefaultCmd_RgbToHsv,
    eCliDefaultCmd_HsvToRgb,
    eCliDefaultCmd_Last
} eCliDefaultCmd;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern sCmdDesc_t g_framework_cli_lut[eCliFrameworkCmd_Last];

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

#endif /* SOURCE_APP_FRAMEWORK_CLI_LUT_H_ */
