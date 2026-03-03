#ifndef SOURCE_APP_DEFAULT_CLI_LUT_H_
#define SOURCE_APP_DEFAULT_CLI_LUT_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_DEFAULT_CMD
#include "cmd_api.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

 /* clang-format off */
typedef enum eCliDefaultCmd {
    eCliDefaultCmd_First = 0,
    
    #ifdef ENABLE_LED
    eCliDefaultCmd_Led_Set,
    eCliDefaultCmd_Led_Reset,
    eCliDefaultCmd_Led_Toggle,
    eCliDefaultCmd_Led_Blink,
    #endif
    
    #ifdef ENABLE_PWM_LED
    eCliDefaultCmd_Pwm_Led_SetBrightness,
    eCliDefaultCmd_Pwm_Led_Pulse,
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

extern sCmdDesc_t g_default_cmd_lut[eCliDefaultCmd_Last];

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

#endif /* ENABLE_DEFAULT_CMD */
#endif /* SOURCE_APP_DEFAULT_CLI_LUT_H_ */
