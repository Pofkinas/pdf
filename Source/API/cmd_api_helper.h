#ifndef SOURCE_API_CMD_API_HELPER_H_
#define SOURCE_API_CMD_API_HELPER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_CMD_HELPER
#include <stddef.h>
#include "message.h"
#include "error_messages.h"

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

eErrorCode_t CMD_API_Helper_ParseToken (char **token, sMessage_t *argument, char *separator, sMessage_t *response); 
eErrorCode_t CMD_API_Helper_FindNextArgUInt (sMessage_t *argument, size_t *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response);
eErrorCode_t CMD_API_Helper_FindNextArgInt (sMessage_t *argument, int *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response);
eErrorCode_t CMD_API_Helper_FindNextArgFloat (sMessage_t *argument, float *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response);
eErrorCode_t CMD_API_Helper_FindNextArgChar (sMessage_t *argument, char *return_argument, char *separator, const size_t separator_lenght, sMessage_t *response);

#endif /* ENABLE_CMD_HELPER */
#endif /* SOURCE_API_CMD_API_HELPER_H_ */
