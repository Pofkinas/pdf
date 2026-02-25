#ifndef SOURCE_APP_CLI_CMD_HANDLERS_H_
#define SOURCE_APP_CLI_CMD_HANDLERS_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_DEFAULT_CMD)
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
    
    #if defined(ENABLE_LED)
    eCliDefaultCmd_Led_Set,
    eCliDefaultCmd_Led_Reset,
    eCliDefaultCmd_Led_Toggle,
    eCliDefaultCmd_Led_Blink,
    #endif /* ENABLE_LED */
    
    #if defined(ENABLE_PWM_LED)
    eCliDefaultCmd_Pwm_LedSetBrightness,
    eCliDefaultCmd_Pwm_LedPulse,
    #endif /* ENABLE_PWM_LED */

    #if defined(ENABLE_MOTOR)
    eCliDefaultCmd_Motors_Set,
    eCliDefaultCmd_Motors_Stop,
    #if defined(ENABLE_PID_CONTROL)
    eCliDefaultCmd_Motors_SetTargetRpm,
    eCliDefaultCmd_Motors_SetPid,
    #endif /* ENABLE_PID_CONTROL */
    #endif  /* ENABLE_MOTOR */
    
    eCliDefaultCmd_RgbToHsv,
    eCliDefaultCmd_HsvToRgb,
    eCliDefaultCmd_Last
} eCliDefaultCmd_t;
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
