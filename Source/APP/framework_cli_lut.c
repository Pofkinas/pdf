/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_cli_lut.h"

#ifdef ENABLE_CLI

#include "cli_cmd_handlers.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEFINE_CMD(command_string) .command = command_string, .command_lenght = sizeof(command_string) - 1

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/* clang-format off */
sCmdDesc_t g_framework_cli_lut[eCliFrameworkCmd_Last] = {
    #ifdef USE_LED
    [eCliFrameworkCmd_Led_Set] = {
        DEFINE_CMD("led_set:"),
        .handler = CLI_APP_Led_Handlers_Set
    },
    [eCliFrameworkCmd_Led_Reset] = {
        DEFINE_CMD("led_reset:"),
        .handler = CLI_APP_Led_Handlers_Reset
    },
    [eCliFrameworkCmd_Led_Toggle] = {
        DEFINE_CMD("led_toggle:"),
        .handler = CLI_APP_Led_Handlers_Toggle
    },
    [eCliFrameworkCmd_Led_Blink] = {
        DEFINE_CMD("led_blink:"),
        .handler = CLI_APP_Led_Handlers_Blink
    },
    #endif

    #ifdef USE_PWM_LED
    [eCliFrameworkCmd_Pwm_Led_SetBrightness] = {
        DEFINE_CMD("led_setb:"),
        .handler = CLI_APP_Pwm_Led_Handlers_Set_Brightness
    },
    [eCliFrameworkCmd_Pwm_Led_Pulse] = {
        DEFINE_CMD("led_pulse:"),
        .handler = CLI_APP_Pwm_Led_Handlers_Pulse
    },
    #endif

    #ifdef USE_MOTORS
    [eCliFrameworkCmd_Motors_Set] = {
        DEFINE_CMD("motors_set:"),
        .handler = CLI_APP_Motors_Handlers_Set
        /* e. g. motors_set:<speed %>, <eMotorDirection_t>, <eMotorControl_t> */
    },
    [eCliFrameworkCmd_Motors_Stop] = {
        DEFINE_CMD("motors_stop"),
        .handler = CLI_APP_Motors_Handlers_Stop
    },
    #endif
    
    [eCliFrameworkCmd_RgbToHsv] = {
        DEFINE_CMD("rgb:"),
        .handler = CLI_APP_Led_Handlers_RgbToHsv
    },
    [eCliFrameworkCmd_HsvToRgb] = {
        DEFINE_CMD("hsv:"),
        .handler = CLI_APP_Led_Handlers_HsvToRgb
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

#endif