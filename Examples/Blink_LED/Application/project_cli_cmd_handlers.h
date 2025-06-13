#ifndef SOURCE_APP_CLI_CMD_HANDLERS_H_
#define SOURCE_APP_CLI_CMD_HANDLERS_H_

/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command handler header file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command handler definition for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * project_cli_cmd_handler.h
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Define your custom commands handlers.
 * 4. Implement the custom commands handlers in `project_cli_cmd_handler.c`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include "message.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Project_CLI_APP_Led_Handlers_Blink (sMessage_t arguments, sMessage_t *response);

#endif /* SOURCE_APP_CLI_APP_HANDLERS_H_ */
