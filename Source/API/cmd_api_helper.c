/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cmd_api_helper.h"

#if defined(ENABLE_CMD_HELPER)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

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

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

eErrorCode_t CMD_API_Helper_ParseToken (char **token, sMessage_t *argument, char *separator, sMessage_t *response) {
    if ((NULL == token) || (NULL == separator) || (NULL == response)) {
        return eErrorCode_NULLPTR;
    }

    if (0 == argument->size) {
        snprintf(response->data, response->size, "Missing argument\n");

        return eErrorCode_ARGFEW;
    }

    *token = strstr(argument->data, separator);
        
    if (NULL != *token) {
        **token = '\0';
    }

    return eErrorCode_OK;
}

eErrorCode_t CMD_API_Helper_FindNextArgUInt (sMessage_t *argument, size_t *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response) {
    char *argument_token = NULL;
    char *invalid_character = NULL;

    eErrorCode_t error = CMD_API_Helper_ParseToken(&argument_token, argument, separator, response);

    if (eErrorCode_OK != error) {
        return error;
    }

    *return_argument = strtoul(argument->data, &invalid_character, BASE_10);

    if ('\0' != *invalid_character) {
        snprintf(response->data, response->size, "[%s]: Invalid argument; Use digits separated by: '%s'\n", invalid_character, separator);

        return eErrorCode_INVAL;
    }

    if (NULL == argument_token) {
        argument->size = 0;
        
        return eErrorCode_OK;
    }

    argument->size -= (argument_token - argument->data + separator_lenght);
    argument->data = argument_token + separator_lenght;

    return eErrorCode_OK;
}

eErrorCode_t CMD_API_Helper_FindNextArgInt (sMessage_t *argument, int *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response) {
    char *argument_token = NULL;
    char *invalid_character = NULL;

    eErrorCode_t error = CMD_API_Helper_ParseToken(&argument_token, argument, separator, response);

    if (eErrorCode_OK != error) {
        return error;
    }

    *return_argument = strtol(argument->data, &invalid_character, BASE_10);

    if ('\0' != *invalid_character) {
        snprintf(response->data, response->size, "[%s]: Invalid argument; Use digits separated by: '%s'\n", invalid_character, separator);

        return eErrorCode_INVAL;
    }

    if (NULL == argument_token) {
        argument->size = 0;
        
        return eErrorCode_OK;
    }

    argument->size -= (argument_token - argument->data + separator_lenght);
    argument->data = argument_token + separator_lenght;

    return eErrorCode_OK;
}

eErrorCode_t CMD_API_Helper_FindNextArgFloat (sMessage_t *argument, float *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response) {
    char *argument_token = NULL;
    char *invalid_character = NULL;

    eErrorCode_t error = CMD_API_Helper_ParseToken(&argument_token, argument, separator, response);

    if (eErrorCode_OK != error) {
        return error;
    }

    *return_argument = strtof(argument->data, &invalid_character);

    if ('\0' != *invalid_character) {
        snprintf(response->data, response->size, "[%s]: Invalid argument; Use float separated by: '%s'\n", invalid_character, separator);

        return eErrorCode_INVAL;
    }

    if (NULL == argument_token) {
        argument->size = 0;
        
        return eErrorCode_OK;
    }

    argument->size -= (argument_token - argument->data + separator_lenght);
    argument->data = argument_token + separator_lenght;

    return eErrorCode_OK;
}

eErrorCode_t CMD_API_Helper_FindNextArgChar (sMessage_t *argument, char *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response) {
    char *argument_token = NULL;

    eErrorCode_t error = CMD_API_Helper_ParseToken(&argument_token, argument, separator, response);

    if (eErrorCode_OK != error) {
        return error;
    }

    *return_argument = argument->data[0];

    if (NULL == argument_token) {
        argument->size = 0;
        
        return eErrorCode_OK;
    }

    argument->size -= (argument_token - argument->data + separator_lenght);
    argument->data = argument_token + separator_lenght;

    return eErrorCode_OK;
}

#endif /* ENABLE_CMD_HELPER */
