#ifndef SOURCE_APP_CLI_CMD_HANDLERS_H_
#define SOURCE_APP_CLI_CMD_HANDLERS_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_DEFAULT_CMD
#include <stdbool.h>
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

#ifdef ENABLE_LED
eErrorCode_t CLI_APP_Led_Handlers_Set (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_APP_Led_Handlers_Reset (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_APP_Led_Handlers_Toggle (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_APP_Led_Handlers_Blink (sMessage_t arguments, sMessage_t *response);
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
eErrorCode_t CLI_APP_Pwm_Led_Handlers_Set_Brightness (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_APP_Pwm_Led_Handlers_Pulse (sMessage_t arguments, sMessage_t *response);
#endif /* ENABLE_PWM_LED */

#ifdef ENABLE_MOTOR
eErrorCode_t CLI_APP_Motors_Handlers_Stop (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_APP_Motors_Handlers_Set (sMessage_t arguments, sMessage_t *response);
#endif /* ENABLE_MOTOR */

eErrorCode_t CLI_APP_Led_Handlers_RgbToHsv (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_APP_Led_Handlers_HsvToRgb (sMessage_t arguments, sMessage_t *response);

#endif /* ENABLE_DEFAULT_CMD */
#endif /* SOURCE_APP_CLI_APP_HANDLERS_H_ */
