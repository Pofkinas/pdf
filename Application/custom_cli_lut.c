
/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command lookup table file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command lookup table implimentation for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * project_cli_lut.c
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Use the `INCLUDE_PROJECT_CLI` macro in `platform_config.h to include custom cli lut in your project.
 * 4. Include the custom commands defined in `project_cli_cmd_handlers.h`.
 * 5. Define your custom commands definitions in `g_framework_cli_lut` sCmdDesc_t.
 * 6. Provide the command handler function for each command in `g_framework_cli_lut`.
 * 7. Implement your custom commands definition header `project_cli_lut.h`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "custom_cli_lut.h"

#ifdef ENABLE_CUSTOM_CMD
#include "custom_cli_cmd_handlers.h"

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
sCmdDesc_t g_custom_cmd_lut[eCliCustomCmd_Last] = {
    [eCliCustomCmd_Led_Blink] = {
        DEFINE_CMD("custom_led_blink:"),
        .handler = Custom_CLI_APP_Led_Handlers_Blink
        /* e. g. custom_led_blink:<eLed_t>, <duration>, <frequency> */
    },
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

#endif /* ENABLE_CUSTOM_CMD */
