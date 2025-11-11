#ifndef SOURCE_UTILITY_ERROR_MESSAGES_H_
#define SOURCE_UTILITY_ERROR_MESSAGES_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eErrorCode {
    eErrorCode_First = 0,

    /* Success */
    eErrorCode_OSOK = eErrorCode_First,        // 0: OK

    /* CLI / user input shape */
    eErrorCode_SYNTAX,                         // 1: Command syntax error
    eErrorCode_ARGFEW,                         // 2: Too few arguments
    eErrorCode_ARGMANY,                        // 3: Too many arguments
    eErrorCode_BADOPT,                         // 4: Unknown/invalid option

    /* Parsing & validation */
    eErrorCode_PARSE,                          // 5: Parse/format error
    eErrorCode_INVAL,                          // 6: Invalid argument value
    eErrorCode_RANGE,                          // 7: Value out of range
    eErrorCode_NULLPTR,                        // 8: Null pointer

    /* State & readiness */
    eErrorCode_BADSTATE,                       // 9: Bad state for operation
    eErrorCode_NOTREADY,                       // 10: Not initialized / not ready

    /* Resources & timing */
    eErrorCode_NOMEM,                          // 11: Out of memory
    eErrorCode_BUSY,                           // 12: Resource busy
    eErrorCode_TIMEOUT,                        // 13: Operation timed out
    eErrorCode_UNAVAIL,                        // 14: Temporarily unavailable

    /* Lookup / existence */
    eErrorCode_NOTFOUND,                       // 15: Item/command not found
    eErrorCode_EXISTS,                         // 16: Already exists

    /* implementation */
    eErrorCode_NOSUP,                          // 17: Not supported
    eErrorCode_NOIMPL,                         // 18: Not implemented

    /* Execution & control */
    eErrorCode_FAILED,                         // 19: Execution failed
    eErrorCode_CANCELED,                       // 20: Operation canceled
    eErrorCode_PERMISSION,                     // 21: Permission denied

    /* Data integrity */
    eErrorCode_DATALOSS,                       // 22: Corrupt/invalid data
    eErrorCode_OVERFLOW,                       // 23: Overflow
    eErrorCode_UNDERFLOW,                      // 24: Underflow

    /* Unexpected internal error */
    eErrorCode_INTERNAL,                       // 25: Internal error
    eErrorCode_UNKWN,                          // 26: Unknown error

    eErrorCode_Last,                         
} eErrorCode_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

const char* Error_Message_To_String(eErrorCode_t error_code);

#endif /* SOURCE_UTILITY_ERROR_MESSAGES_H_ */
