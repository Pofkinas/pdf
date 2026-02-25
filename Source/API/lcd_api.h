#ifndef SOURCE_API_LCD_API_H_
#define SOURCE_API_LCD_API_H_
/***********************************************************************************************************************
 * @note Current LCD API requires I2C API and osDelay that is based on RTOS. Make sure to initialize LCD after
 * the RTOS scheduler has started.
 * @todo: Refactor osDelay with a timer-based delay
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_LCD)
#include <stdbool.h>
#include <stdint.h>
#include "message.h"
#include "lcd_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define COMMAND_HIGH 0x0C
#define COMMAND_LOW 0x08
#define DATA_HIGH 0x0D
#define DATA_LOW 0x09

// HD44780 LCD commands
#define LCD_CLEAR_DISPLAY         0x01  // Clear display and return home
#define LCD_RETURN_HOME           0x02  // Return cursor to home position

// Entry Mode Set
#define LCD_ENTRY_MODE_SET        0x04
#define LCD_ENTRY_INCREMENT       0x02  // I/D bit: 1 = increment cursor
#define LCD_ENTRY_DECREMENT       0x00  // I/D bit: 0 = decrement cursor
#define LCD_ENTRY_SHIFT_ON        0x01  // S bit: 1 = shift display
#define LCD_ENTRY_SHIFT_OFF       0x00  // S bit: 0 = no shift

// Display Control
#define LCD_DISPLAY_CONTROL       0x08
#define LCD_DISPLAY_ON            0x04  // D=1 (display on)
#define LCD_DISPLAY_OFF           0x00  // D=0 (display off)
#define LCD_CURSOR_ON             0x02  // C=1 (cursor on)
#define LCD_CURSOR_OFF            0x00  // C=0 (cursor off)
#define LCD_BLINK_ON              0x01  // B=1 (blinking on)
#define LCD_BLINK_OFF             0x00  // B=0 (blinking off)

// Function Set
#define LCD_FUNCTION_SET          0x20
#define LCD_FUNCTION_8BIT         0x10  // DL=1 (8-bit mode)
#define LCD_FUNCTION_4BIT         0x00  // DL=0 (4-bit mode)
#define LCD_FUNCTION_2LINE        0x08  // N=1 (2 lines)
#define LCD_FUNCTION_1LINE        0x00  // N=0 (1 line)
#define LCD_FUNCTION_5x10DOTS     0x04  // F=1 (5x10 font)
#define LCD_FUNCTION_5x8DOTS      0x00  // F=0 (5x8 font)

// Set DDRAM Address (cursor position)
#define LCD_SET_DDRAM_ADDR        0x80

// Wake-up command
#define LCD_WAKEUP_COMMAND        0x30  /* 8-bit wakeup sequence */

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

bool LCD_API_InitAll (void);
bool LCD_API_Clear (const eLcd_t lcd);
bool LCD_API_TurnOn (const eLcd_t lcd);
bool LCD_API_TurnOff (const eLcd_t lcd);
bool LCD_API_SendCommand (const eLcd_t lcd, const uint8_t command);
bool LCD_API_Print (const eLcd_t lcd, const sMessage_t *message, const eLcdRow_t row, const eLcdColumn_t column, const eLcdOption_t option);

#endif /* ENABLE_LCD */
#endif /* SOURCE_API_LCD_API_H_ */
