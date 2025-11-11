#ifndef SOURCE_API_DEBUG_API_H_
#define SOURCE_API_DEBUG_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_UART_DEBUG
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "baudrate.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define CREATE_MODULE_NAME(file_name) static const char *trace_module_name = #file_name;
#define CREATE_MODULE_NAME_EMPTY static const char *trace_module_name __attribute__((unused)) = NULL;

#ifdef ENABLE_UART_DEBUG
#define TRACE_INFO(format, ...) Debug_API_Print(eTraceLevel_Info, trace_module_name,__FILE__, __LINE__, format, ##__VA_ARGS__)
#define TRACE_WRN(format, ...) Debug_API_Print(eTraceLevel_Warning, trace_module_name,__FILE__, __LINE__, format, ##__VA_ARGS__)
#define TRACE_ERR(format, ...) Debug_API_Print(eTraceLevel_Error, trace_module_name, __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define TRACE_INFO(format, ...)
#define TRACE_WRN(format, ...)
#define TRACE_ERR(format, ...)
#endif

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eTraceLevel {
    eTraceLevel_First = 0,
    eTraceLevel_Info = eTraceLevel_First,
    eTraceLevel_Warning,
    eTraceLevel_Error,
    eTraceLevel_Last
} eTraceLevel_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Debug_API_Init (const eBaudrate_t baudrate);
bool Debug_API_Print (const eTraceLevel_t trace_level, const char *file_trace, const char *file_name, const size_t line_number, const char *format, ...);

#endif /* ENABLE_UART_DEBUG */
#endif /* SOURCE_API_DEBUG_API_H_ */
