#ifndef SOURCE_APP_CLI_CMD_H_
#define SOURCE_APP_CLI_CMD_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_DEFAULT_CMD)
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

#if defined(ENABLE_LED)
eErrorCode_t CLI_CMD_Led_Set (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_CMD_Led_Reset (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_CMD_Led_Toggle (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_CMD_Led_Blink (sMessage_t arguments, sMessage_t *response);
#endif /* ENABLE_LED */

#if defined(ENABLE_PWM_LED)
eErrorCode_t CLI_CMD_Pwm_LedSetBrightness (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_CMD_Pwm_LedPulse (sMessage_t arguments, sMessage_t *response);
#endif /* ENABLE_PWM_LED */

#if defined(ENABLE_MOTOR)
eErrorCode_t CLI_CMD_Motors_Stop (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_CMD_Motors_Set (sMessage_t arguments, sMessage_t *response);
#if defined(ENABLE_PID_CONTROL)
eErrorCode_t CLI_CMD_Motors_SetTargetRpm (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_CMD_Motors_SetPid (sMessage_t arguments, sMessage_t *response);
#endif /* ENABLE_PID_CONTROL */
#endif /* ENABLE_MOTOR */

eErrorCode_t CLI_CMD_Led_RgbToHsv (sMessage_t arguments, sMessage_t *response);
eErrorCode_t CLI_CMD_Led_HsvToRgb (sMessage_t arguments, sMessage_t *response);

#endif /* ENABLE_DEFAULT_CMD */
#endif /* SOURCE_APP_CLI_CMD_H_ */
