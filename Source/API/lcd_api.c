/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "lcd_api.h"

#if defined(ENABLE_LCD)
#include "cmsis_os2.h"
#include "debug_api.h"
#include "i2c_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sLcdByte {
    uint8_t upper_4bits;
    uint8_t lower_4bits;
} sLcdByte_t;

typedef enum eLcdTxMode {
    eLcdTxMode_First = 0,
    eLcdTxMode_Command = eLcdTxMode_First,
    eLcdTxMode_Data,
    eLcdTxMode_Last
} eLcdTxMode_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#if defined(DEBUG_LCD_API)
CREATE_MODULE_NAME (LCD_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_LCD_API */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_lcd_initialized = false;

static sLcdDesc_t g_static_lcd_lut[eLcd_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool LCD_API_Send (const eLcd_t lcd, const uint8_t command, const eLcdTxMode_t tx_mode);
static sLcdByte_t LCD_API_ConvertToLcdByte (const uint8_t data);
static bool LCD_API_WakeUpDisplay (const eLcd_t lcd);
static bool LCD_API_InitDisplay (const eLcd_t lcd);
static bool LCD_API_SetCursor (const eLcd_t lcd, const eLcdRow_t row, const eLcdColumn_t column);
static bool LCD_API_SendBytes (const eLcd_t lcd, const char *data, const size_t data_size, const eLcdTxMode_t tx_mode);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static bool LCD_API_Send (const eLcd_t lcd, const uint8_t data, const eLcdTxMode_t tx_mode) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("Send: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }
    
    sLcdByte_t lcd_byte = LCD_API_ConvertToLcdByte(data);

    uint8_t send_data[4];

    send_data[0] = lcd_byte.upper_4bits | ((eLcdTxMode_Command == tx_mode) ? COMMAND_HIGH : DATA_HIGH);
    send_data[1] = lcd_byte.upper_4bits | ((eLcdTxMode_Command == tx_mode) ? COMMAND_LOW : DATA_LOW);
    send_data[2] = lcd_byte.lower_4bits | ((eLcdTxMode_Command == tx_mode) ? COMMAND_HIGH : DATA_HIGH);
    send_data[3] = lcd_byte.lower_4bits | ((eLcdTxMode_Command == tx_mode) ? COMMAND_LOW : DATA_LOW);

    return I2C_API_Write(g_static_lcd_lut[lcd].i2c, g_static_lcd_lut[lcd].i2c_address, send_data, sizeof(send_data), 0, 0, LCD_I2C_TIMEOUT);
}

static sLcdByte_t LCD_API_ConvertToLcdByte (const uint8_t data) {
    sLcdByte_t lcd_byte = {0};
    
    lcd_byte.upper_4bits = data & 0xF0;
    lcd_byte.lower_4bits = (data << 4) & 0xF0;
    
    return lcd_byte;
}

static bool LCD_API_WakeUpDisplay (const eLcd_t lcd) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("WakeUpDisplay: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    bool is_wakeup_successful = true;

    if (!LCD_API_Send(lcd, LCD_WAKEUP_COMMAND, eLcdTxMode_Command)) {
        TRACE_ERR("WakeUpDisplay: Failed to send wakeup command LCD [%d]\n", lcd);
        
        is_wakeup_successful = false;
    }

    osDelay(5);

    if (!LCD_API_Send(lcd, LCD_WAKEUP_COMMAND, eLcdTxMode_Command)) {
        TRACE_ERR("WakeUpDisplay: Failed to send second wakeup command LCD [%d]\n", lcd);
        
        is_wakeup_successful = false;
    }

    osDelay(1);

    if (!LCD_API_Send(lcd, LCD_WAKEUP_COMMAND, eLcdTxMode_Command)) {
        TRACE_ERR("WakeUpDisplay: Failed to send third wakeup command LCD [%d]\n", lcd);
        
        is_wakeup_successful = false;
    }

    osDelay(1);

    return is_wakeup_successful;
}

static bool LCD_API_InitDisplay (const eLcd_t lcd) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("InitDisplay: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    bool is_init_successful = true;

    uint8_t command = (LCD_FUNCTION_SET | LCD_FUNCTION_4BIT | LCD_FUNCTION_1LINE | LCD_FUNCTION_5x8DOTS);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("InitDisplay: Failed to init 4-bit mode (%x) LCD [%d]\n", command, lcd);
    
        is_init_successful = false;
    }

    command = (LCD_FUNCTION_SET | LCD_FUNCTION_4BIT | LCD_FUNCTION_2LINE | LCD_FUNCTION_5x8DOTS);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("InitDisplay: Failed to init 2-line mode (%x) LCD [%d]\n", command, lcd);
        
        is_init_successful = false;
    }

    command = (LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINK_OFF);

    if (!LCD_API_Send(lcd, (LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINK_OFF), eLcdTxMode_Command)) {
        TRACE_ERR("InitDisplay: Failed to turn off display (%x) LCD [%d]\n", command, lcd);
        
        is_init_successful = false;
    }

    if (!LCD_API_Send(lcd, LCD_CLEAR_DISPLAY, eLcdTxMode_Command)) {
        TRACE_ERR("InitDisplay: Failed to clear display LCD [%d]\n", lcd);
        
        is_init_successful = false;
    }

    osDelay(2);

    command = (LCD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT | LCD_ENTRY_SHIFT_OFF);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("InitDisplay: Failed to set entry mode (%x) LCD [%d]\n", command, lcd);
        
        is_init_successful = false;
    }

    command = (LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("InitDisplay: Failed to turn off display (%x) LCD [%d]\n", command, lcd);
        
        is_init_successful = false;
    }

    return is_init_successful;
}

static bool LCD_API_SetCursor (const eLcd_t lcd, const eLcdRow_t row, const eLcdColumn_t column) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("SetCursor: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    uint8_t command = (LCD_SET_DDRAM_ADDR | (g_static_lcd_lut[lcd].row_addresses[row] + column));

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("SetCursor: Failed to set cursor (%d, %d) LCD [%d]\n", row, column, lcd);
        
        return false;
    }

    return true;
}

static bool LCD_API_SendBytes (const eLcd_t lcd, const char *data, const size_t data_size, const eLcdTxMode_t tx_mode) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("SendBytes: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    if ((NULL == data) || (0 == data_size)) {
        TRACE_ERR("SendBytes: Invalid data or size\n");
        
        return false;
    }

    for (size_t byte = 0; byte < data_size; byte++) {
        if (!LCD_API_Send(lcd, data[byte], tx_mode)) {
            return false;
        }
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool LCD_API_InitAllLcd (void) {
    if (g_is_lcd_initialized) {
        return true;
    }

    osDelay(50);

    g_is_lcd_initialized = true;

    for (eLcd_t lcd = eLcd_First; lcd < eLcd_Last; lcd++) {
        const sLcdDesc_t *desc = LCD_Config_GetLcdDesc(lcd);

        if (NULL == desc) {
            TRACE_ERR("InitAllLcd: Failed to get LCD [%d] description\n", lcd);

            g_is_lcd_initialized = false;
            
            return false;
        }

        g_static_lcd_lut[lcd] = *desc;

        if (!I2C_API_Init(g_static_lcd_lut[lcd].i2c)) {
            TRACE_ERR("InitAllLcd: Failed to initialize I2C for LCD [%d]\n", lcd);

            g_is_lcd_initialized = false;
            
            return false;
        }
        
        if (!LCD_API_WakeUpDisplay(lcd)) {
            g_is_lcd_initialized = false;
        }

        if (!LCD_API_InitDisplay(lcd)) {
            g_is_lcd_initialized = false;
        }
    }

    return g_is_lcd_initialized;
}

bool LCD_API_Clear (const eLcd_t lcd) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("Clear: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    if (eLcdState_Init != g_lcd_state[lcd]) {
        TRACE_ERR ("Clear: LCD not initialized [%d]\n", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, LCD_CLEAR_DISPLAY, eLcdTxMode_Command)) {
        TRACE_ERR("Clear: Failed to clear display for LCD [%d]\n", lcd);
        
        return false;
    }

    osDelay(2);

    return true;
}

bool LCD_API_TurnOn (const eLcd_t lcd) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("TurnOn: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }
    
    if (eLcdState_Init != g_lcd_state[lcd]) {
        TRACE_ERR ("TurnOn: LCD not initialized [%d]\n", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, (LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON), eLcdTxMode_Command)) {
        TRACE_ERR("TurnOn: Failed to turn on display for LCD [%d]\n", lcd);
        
        return false;
    }

    return true;
}

bool LCD_API_TurnOff (const eLcd_t lcd) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("TurnOff: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    if (eLcdState_Init != g_lcd_state[lcd]) {
        TRACE_ERR ("TurnOff: LCD not initialized [%d]\n", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, (LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF), eLcdTxMode_Command)) {
        TRACE_ERR("TurnOff: Failed to turn off display for LCD [%d]\n", lcd);
        
        return false;
    }

    return true;
}

bool LCD_API_SendCommand (const eLcd_t lcd, const uint8_t command) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("SendCommand: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    if (eLcdState_Init != g_lcd_state[lcd]) {
        TRACE_ERR ("SendCommand: LCD not initialized [%d]\n", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("SendCommand: Failed to send command (%x) to LCD [%d]\n", command, lcd);
        
        return false;
    }

    return true;
}

bool LCD_API_Print (const eLcd_t lcd, const sMessage_t *message, const eLcdRow_t row, const eLcdColumn_t column, const eLcdOption_t option) {
    if (!LCD_Config_IsCorrectLcd(lcd)) {
        TRACE_ERR ("Print: Incorrect LCD type [%d]\n", lcd);
        
        return false;
    }

    if ((NULL == message) || (NULL == message->data) || (0 == message->size)) {
        TRACE_ERR("Print: Invalid message data\n");
        
        return false;
    }

    if ((row < eLcdRow_First) || (row > g_static_lcd_lut[lcd].rows)) {
        TRACE_ERR("Print: Invalid row [%d]\n", row);
        
        return false;
    }

    if ((column < eLcdColumn_First) || (column > g_static_lcd_lut[lcd].columns)) {
        TRACE_ERR("Print: Invalid column [%d]\n", column);
        
        return false;
    }

    if (eLcdState_Init != g_lcd_state[lcd]) {
        TRACE_ERR ("Print: LCD not initialized [%d]\n", lcd);
        
        return false;
    }

    switch (option) {
        case eLcdOption_None: {} break;
        case eLcdOption_Refresh: {
            if (!LCD_API_Clear(lcd)) {
                return false;
            }
        } break;
        default: {
            TRACE_ERR("Print: Invalid option [%d]\n", option);
            
            return false;
        }
    }

    if (!LCD_API_SetCursor(lcd, row, column)) {
        return false;
    }

    return LCD_API_SendBytes(lcd, message->data, message->size, eLcdTxMode_Data);
}

#endif /* ENABLE_LCD */
