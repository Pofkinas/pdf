/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cmd_api.h"

#ifdef ENABLE_CLI
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "debug_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_CMD_API
CREATE_MODULE_NAME (CMD_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_CMD_API */

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

eErrorCode_t CMD_API_FindCommand (sMessage_t command, sMessage_t *response, sCmdDesc_t *command_lut, const size_t command_lut_size) {
    if ((response == NULL) || (command_lut == NULL)) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (response->data == NULL) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }
    
    for (size_t command_number = 1; command_number < command_lut_size; command_number++) {
        if (strncmp(command.data, command_lut[command_number].command, command_lut[command_number].command_length) != 0) {
            continue;
        }

        command.data += command_lut[command_number].command_length;
        command.size -= command_lut[command_number].command_length;

        return command_lut[command_number].handler(command, response);
    }

    snprintf(response->data, response->size, "Invalid command\n");

    return eErrorCode_NOTFOUND;
}

#endif /* ENABLE_CLI */
