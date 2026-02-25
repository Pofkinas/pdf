/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "error_messages.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

const static char* g_static_error_messages [eErrorCode_Last] = {
    [eErrorCode_OK]         = "OK",

    [eErrorCode_SYNTAX]     = "Syntax error",
    [eErrorCode_ARGFEW]     = "Too few args",
    [eErrorCode_ARGMANY]    = "Too many args",
    [eErrorCode_BADOPT]     = "Bad option",

    [eErrorCode_PARSE]      = "Parse error",
    [eErrorCode_INVAL]      = "Invalid argument",
    [eErrorCode_RANGE]      = "Out of range",
    [eErrorCode_NULLPTR]    = "Null pointer",

    [eErrorCode_BADSTATE]   = "Bad state",
    [eErrorCode_NOTREADY]   = "Not ready",

    [eErrorCode_NOMEM]      = "Out of memory",
    [eErrorCode_TIMEOUT]    = "Timeout",
    [eErrorCode_UNAVAIL]    = "Unavailable",

    [eErrorCode_NOTFOUND]   = "Not found",
    [eErrorCode_EXISTS]     = "Already exists",

    [eErrorCode_NOSUP]      = "Not supported",
    [eErrorCode_NOIMPL]     = "Not implemented",

    [eErrorCode_FAILED]     = "Execution failed",
    [eErrorCode_CANCELED]   = "Canceled",
    [eErrorCode_PERMISSION] = "Permission denied",

    [eErrorCode_DATALOSS]   = "Data loss",
    [eErrorCode_OVERFLOW]   = "Overflow",
    [eErrorCode_UNDERFLOW]  = "Underflow",

    [eErrorCode_INTERNAL]   = "Internal error",
    [eErrorCode_UNKWN]      = "Unknown error"
};

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

const char* Error_Message_To_String(eErrorCode_t error_code) {
    if ((error_code < eErrorCode_First) || (error_code >= eErrorCode_Last)) {
        return g_static_error_messages[eErrorCode_UNKWN];
    }
    
    return g_static_error_messages[error_code];
}
