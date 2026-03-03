#ifndef SOURCE_API_LCD_API_H_
#define SOURCE_API_LCD_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_LCD
#include <stdbool.h>
#include <stdint.h>
#include "message.h"
#include "lcd_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eLcdOption {
    eLcdOption_First = 0,
    eLcdOption_None = eLcdOption_First, 
    eLcdOption_Refresh,
    eLcdOption_Last
} eLcdOption_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool LCD_API_InitAllLcd (void);
bool LCD_API_Clear (const eLcd_t lcd);
bool LCD_API_TurnOn (const eLcd_t lcd);
bool LCD_API_TurnOff (const eLcd_t lcd);
bool LCD_API_SendCommand (const eLcd_t lcd, const uint8_t command);
bool LCD_API_Print (const eLcd_t lcd, const sMessage_t *message, const eLcdRow_t row, const eLcdColumn_t column, const eLcdOption_t option);

#endif /* ENABLE_LCD */
#endif /* SOURCE_API_LCD_API_H_ */
