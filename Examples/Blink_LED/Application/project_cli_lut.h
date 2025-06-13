#ifndef APPLICATION_PROJECT_CLI_LUT_H_
#define APPLICATION_PROJECT_CLI_LUT_H_

/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command lookup table header file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command lookup table definition for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * project_cli_lut.h
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Use the `INCLUDE_PROJECT_CLI` macro in `platform_config.h` to include custom cli lut in your project.
 * 4. Define your custom commands in `eCliProjectCmd` enum.
 * 5. Implement the custom commands definitions in `project_cli_lut.c`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cmd_api.h"
#include "platform_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eCliProjectCmd {
    eCliProjectCmd_First = 0,
    
    #ifdef INCLUDE_PROJECT_CLI
    eCliProjectCmd_Led_Blink,
    #endif

    eCliProjectCmd_Last
} eCliProjectCmd;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern sCmdDesc_t g_project_cli_lut[eCliProjectCmd_Last];

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

#endif /* APPLICATION_PROJECT_CLI_LUT_H_ */
