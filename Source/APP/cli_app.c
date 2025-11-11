/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cli_app.h"

#ifdef ENABLE_CLI

#include <ctype.h>
#include "cmsis_os2.h"
#include "default_cli_lut.h"
#include "platform_config.h"
#include "cmd_api.h"
#include "uart_api.h"
#include "heap_api.h"
#include "debug_api.h"
#include "message.h"
#include "error_messages.h"

#ifdef ENABLE_CUSTOM_CMD
#include "custom_cli_lut.h"
#endif

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_CLI_APP
CREATE_MODULE_NAME (CLI_APP)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_CLI_APP */

const static osThreadAttr_t g_cli_thread_attributes = {
    .name = "CLI_APP_Thread",
    .stack_size = CLI_APP_THREAD_STACK_SIZE,
    .priority = (osPriority_t) CLI_APP_THREAD_PRIORITY
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_initialized = false;

static osThreadId_t g_cli_thread_id = NULL;
static char g_response_buffer[RESPONSE_MESSAGE_CAPACITY];

static sMessage_t g_command = {.data = NULL, .size = 0};
static sMessage_t g_response = {.data = g_response_buffer, .size = RESPONSE_MESSAGE_CAPACITY};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void CLI_APP_Thread (void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void CLI_APP_Thread (void *arg) {
    while (true) {
        if (UART_API_Receive(DEBUG_UART, &g_command, osWaitForever)) {
            eErrorCode_t error_code = eErrorCode_NOTFOUND;
            
            #ifdef ENABLE_DEFAULT_CMD
            error_code = CMD_API_FindCommand(g_command, &g_response, g_default_cmd_lut, eCliDefaultCmd_Last);
            #endif /* ENABLE_DEFAULT_CMD */

            #ifdef ENABLE_CUSTOM_CMD
            if (error_code == eErrorCode_NOTFOUND) {
                error_code = CMD_API_FindCommand(g_command, &g_response, g_custom_cmd_lut, eCliCustomCmd_Last);
            }
            #endif /* ENABLE_CUSTOM_CMD */

            if (error_code != eErrorCode_OSOK) {
                TRACE_WRN(g_response.data);
            }
            
            Heap_API_Free(g_command.data);
        }
    }

    osThreadYield();
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool CLI_APP_Init (const eBaudrate_t baudrate) {
    if (g_is_initialized) {
        return true;
    }

    if ((baudrate < eBaudrate_First) || (baudrate >= eBaudrate_Last)) {
        return false;
    }

    if (Heap_API_Init() == false) {
        return false;
    }

    if (Debug_API_Init(baudrate) == false) {
        return false;
    }

    g_cli_thread_id = osThreadNew(CLI_APP_Thread, NULL, &g_cli_thread_attributes);

    if (g_cli_thread_id == NULL) {
        return false;
    }

    g_is_initialized = true;

    return g_is_initialized;
}

#endif /* ENABLE_CLI */
