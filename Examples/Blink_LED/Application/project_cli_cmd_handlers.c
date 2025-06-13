
/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command handler file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command handler implimentation for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * project_cli_cmd_handler.c
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Include the used module headers.
 * 4. Define command seperator symbol (e.g. `,`) and seperator length (e.g. `sizeof(SEPARATOR) - 1`).
 * 5. Imlement the custom commands.
 * 6. Use `CMD_API_Helper_FindNextArgUInt` to parse the command arguments.
 * 7. Implement the custom commands handler definition header `project_cli_cmd_handler.h`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "project_cli_cmd_handlers.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "led_app.h"
#include "cmd_api_helper.h"
#include "heap_api.h"
#include "led_api.h"
#include "debug_api.h"
#include "error_messages.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEBUG_PROJECT_CLI_APP

#define LED_SEPARATOR ","
#define LED_SEPARATOR_LENGHT (sizeof(LED_SEPARATOR) - 1)

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_PROJECT_CLI_APP
CREATE_MODULE_NAME (PROJECT_CLI_CMD_HANDLERS)
#else
CREATE_MODULE_NAME_EMPTY
#endif

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Project_CLI_APP_Led_Handlers_Blink (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }
    
    eLed_t led;
    size_t led_value = 0;
    size_t blink_time = 0;
    size_t blink_frequency = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, LED_SEPARATOR, LED_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &blink_time, LED_SEPARATOR, LED_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &blink_frequency, LED_SEPARATOR, LED_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    if (!LED_API_IsCorrectBlinkTime(blink_time)) {
        snprintf(response->data, response->size, "%d: Incorrect blink time\n", blink_time);

        return false;
    }

    if (!LED_API_IsCorrectBlinkFrequency(blink_frequency)) {
        snprintf(response->data, response->size, "%d: Incorrect blink frequency\n", blink_frequency);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Blink, .data = NULL};
    sLedBlink_t *task_data = Heap_API_Calloc(1, sizeof(sLedBlink_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led = led;
    task_data->blink_time = blink_time;
    task_data->blink_frequency = blink_frequency;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}
