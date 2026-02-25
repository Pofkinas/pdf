/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "default_cli_lut.h"

#if defined(ENABLE_DEFAULT_CMD)

#include "cli_cmd.h"

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
    #if defined(ENABLE_LED)
    [eCliDefaultCmd_Led_Set] = {
        DEFINE_CMD("led_set:"),
        .handler = CLI_CMD_Led_Set
        /* e. g. led_set:<eLed_t> */
    },
    [eCliDefaultCmd_Led_Reset] = {
        DEFINE_CMD("led_reset:"),
        .handler = CLI_CMD_Led_Reset
        /* e. g. led_reset:<eLed_t> */
    },
    [eCliDefaultCmd_Led_Toggle] = {
        DEFINE_CMD("led_toggle:"),
        .handler = CLI_CMD_Led_Toggle
        /* e. g. led_toggle:<eLed_t> */
    },
    [eCliDefaultCmd_Led_Blink] = {
        DEFINE_CMD("led_blink:"),
        .handler = CLI_CMD_Led_Blink
        /* e. g. led_blink:<eLed_t>, <duration>, <frequency> */
    },
    #endif /* ENABLE_LED */

    #if defined(ENABLE_PWM_LED)
    [eCliDefaultCmd_Pwm_LedSetBrightness] = {
        DEFINE_CMD("led_setb:"),
        .handler = CLI_CMD_Pwm_LedSetBrightness
        /* e. g. led_setb:<eLedPwm_t>, <duty_cycle> */
    },
    [eCliDefaultCmd_Pwm_LedPulse] = {
        DEFINE_CMD("led_pulse:"),
        .handler = CLI_CMD_Pwm_LedPulse
        /* e. g. led_pulse:<eLedPwm_t>, <pulse_time>, <pulse_frequency> */
    },
    #endif /* ENABLE_PWM_LED */

    #if defined(ENABLE_MOTOR)
    [eCliDefaultCmd_Motors_Set] = {
        DEFINE_CMD("motors_set:"),
        .handler = CLI_CMD_Motors_Set
        /* e. g. motors_set:<speed %>, <eMotorDirection_t>, <eMotorControl_t> */
    },
    [eCliDefaultCmd_Motors_Stop] = {
        DEFINE_CMD("motors_stop"),
        .handler = CLI_CMD_Motors_Stop
        /* e. g. motors_stop */
    },
    #if defined(ENABLE_PID_CONTROL)
    [eCliDefaultCmd_Motors_SetTargetRpm] = {
        DEFINE_CMD("motors_setrpm:"),
        .handler = CLI_CMD_Motors_SetTargetRpm
        /* e. g. motors_setrpm:<eMotor_t>, <target_rpm>, <eMotorControl_t> */
    },
    [eCliDefaultCmd_Motors_SetPid] = {
        DEFINE_CMD("motors_setpid:"),
        .handler = CLI_CMD_Motors_SetPid
        /* e. g. motors_setpid:<eMotor_t>, <Kp>, <Ki>, <Kd> */
    },
    #endif /* ENABLE_PID_CONTROL */
    #endif /* ENABLE_MOTOR */
    
    [eCliDefaultCmd_RgbToHsv] = {
        DEFINE_CMD("rgb:"),
        .handler = CLI_CMD_Led_RgbToHsv
        /* e. g. rgb:<r>, <g>, <b> */
    },
    [eCliDefaultCmd_HsvToRgb] = {
        DEFINE_CMD("hsv:"),
        .handler = CLI_CMD_Led_HsvToRgb
        /* e. g. hsv:<h>, <s>, <v> */
    }
    // TODO: Add VL53L0X calibration
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
