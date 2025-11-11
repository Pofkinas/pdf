/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "default_cli_lut.h"

#ifdef ENABLE_DEFAULT_CMD

#include "cli_cmd_handlers.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEFINE_CMD(command_string) .command = command_string, .command_length = sizeof(command_string) - 1

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
sCmdDesc_t g_default_cmd_lut[eCliDefaultCmd_Last] = {
    #ifdef ENABLE_LED
    [eCliDefaultCmd_Led_Set] = {
        DEFINE_CMD("led_set:"),
        .handler = CLI_APP_Led_Handlers_Set
        /* e. g. led_set:<eLed_t> */
    },
    [eCliDefaultCmd_Led_Reset] = {
        DEFINE_CMD("led_reset:"),
        .handler = CLI_APP_Led_Handlers_Reset
        /* e. g. led_reset:<eLed_t> */
    },
    [eCliDefaultCmd_Led_Toggle] = {
        DEFINE_CMD("led_toggle:"),
        .handler = CLI_APP_Led_Handlers_Toggle
        /* e. g. led_toggle:<eLed_t> */
    },
    [eCliDefaultCmd_Led_Blink] = {
        DEFINE_CMD("led_blink:"),
        .handler = CLI_APP_Led_Handlers_Blink
        /* e. g. led_blink:<eLed_t>, <duration>, <frequency> */
    },
    #endif

    #ifdef ENABLE_PWM_LED
    [eCliDefaultCmd_Pwm_Led_SetBrightness] = {
        DEFINE_CMD("led_setb:"),
        .handler = CLI_APP_Pwm_Led_Handlers_Set_Brightness
        /* e. g. led_setb:<eLedPwm_t>, <duty_cycle> */
    },
    [eCliDefaultCmd_Pwm_Led_Pulse] = {
        DEFINE_CMD("led_pulse:"),
        .handler = CLI_APP_Pwm_Led_Handlers_Pulse
        /* e. g. led_pulse:<eLedPwm_t>, <pulse_time>, <pulse_frequency> */
    },
    #endif

    #ifdef ENABLE_MOTOR
    [eCliDefaultCmd_Motors_Set] = {
        DEFINE_CMD("motors_set:"),
        .handler = CLI_APP_Motors_Handlers_Set
        /* e. g. motors_set:<speed %>, <eMotorDirection_t> */
    },
    [eCliDefaultCmd_Motors_Stop] = {
        DEFINE_CMD("motors_stop"),
        .handler = CLI_APP_Motors_Handlers_Stop
        /* e. g. motors_stop */
    },
    #endif
    
    [eCliDefaultCmd_RgbToHsv] = {
        DEFINE_CMD("rgb:"),
        .handler = CLI_APP_Led_Handlers_RgbToHsv
        /* e. g. rgb:<r>, <g>, <b> */
    },
    [eCliDefaultCmd_HsvToRgb] = {
        DEFINE_CMD("hsv:"),
        .handler = CLI_APP_Led_Handlers_HsvToRgb
        /* e. g. hsv:<h>, <s>, <v> */
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

#endif /* ENABLE_DEFAULT_CMD */
