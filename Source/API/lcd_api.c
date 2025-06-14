/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "lcd_api.h"

#ifdef USE_LCD

#include "cmsis_os2.h"
#include "i2c_api.h"
#include "debug_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define LCD_I2C_TIMEOUT 50

#define FIRST_ROW_ADDRESS 0x00
#define SECOND_ROW_ADDRESS 0x40

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sLcdDesc {
    eI2c_t i2c;
    uint8_t i2c_address;
} sLcdDesc_t;

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

#ifdef DEBUG_LCD_API
CREATE_MODULE_NAME (DEBUG_LCD_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif

static const uint8_t g_row_offset[] = {FIRST_ROW_ADDRESS, SECOND_ROW_ADDRESS};

/* clang-format off */
static const sLcdDesc_t g_static_lcd_lut[eLcd_Last] = {
    #ifdef USE_LCD_1
    [eLcd_1] = {
        .i2c = eI2c_1,
        .i2c_address = 0x27,
    },
    #endif
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
static bool g_is_lcd_initialized = false;

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
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }
    
    sLcdByte_t lcd_byte = LCD_API_ConvertToLcdByte(data);

    uint8_t send_data[4];

    send_data[0] = lcd_byte.upper_4bits | ((tx_mode == eLcdTxMode_Command) ? COMMAND_HIGH : DATA_HIGH);
    send_data[1] = lcd_byte.upper_4bits | ((tx_mode == eLcdTxMode_Command) ? COMMAND_LOW : DATA_LOW);
    send_data[2] = lcd_byte.lower_4bits | ((tx_mode == eLcdTxMode_Command) ? COMMAND_HIGH : DATA_HIGH);
    send_data[3] = lcd_byte.lower_4bits | ((tx_mode == eLcdTxMode_Command) ? COMMAND_LOW : DATA_LOW);

    return I2C_API_Write(g_static_lcd_lut[lcd].i2c, g_static_lcd_lut[lcd].i2c_address, send_data, sizeof(send_data), 0, 0, LCD_I2C_TIMEOUT);
}

static sLcdByte_t LCD_API_ConvertToLcdByte (const uint8_t data) {
    sLcdByte_t lcd_byte = {0};
    
    lcd_byte.upper_4bits = data & 0xF0;
    lcd_byte.lower_4bits = (data << 4) & 0xF0;
    
    return lcd_byte;
}

static bool LCD_API_WakeUpDisplay (const eLcd_t lcd) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }

    bool is_wakeup_successful = true;

    if (!LCD_API_Send(lcd, LCD_WAKEUP_COMMAND, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_WakeUpDisplay: Failed to send wakeup command LCD %d", lcd);
        
        is_wakeup_successful = false;
    }

    osDelay(5);

    if (!LCD_API_Send(lcd, LCD_WAKEUP_COMMAND, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_WakeUpDisplay: Failed to send second wakeup command LCD %d", lcd);
        
        is_wakeup_successful = false;
    }

    osDelay(1);

    if (!LCD_API_Send(lcd, LCD_WAKEUP_COMMAND, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_WakeUpDisplay: Failed to send third wakeup command LCD %d", lcd);
        
        is_wakeup_successful = false;
    }

    osDelay(1);

    return is_wakeup_successful;
}

static bool LCD_API_InitDisplay (const eLcd_t lcd) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }

    bool is_init_successful = true;

    uint8_t command = (LCD_FUNCTION_SET | LCD_FUNCTION_4BIT | LCD_FUNCTION_1LINE | LCD_FUNCTION_5x8DOTS);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_InitDisplay: Failed to init 4-bit mode (%x) LCD %d", command, lcd);
    
        is_init_successful = false;
    }

    command = (LCD_FUNCTION_SET | LCD_FUNCTION_4BIT | LCD_FUNCTION_2LINE | LCD_FUNCTION_5x8DOTS);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_InitDisplay: Failed to init 2-line mode (%x) LCD %d", command, lcd);
        
        is_init_successful = false;
    }

    command = (LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINK_OFF);

    if (!LCD_API_Send(lcd, (LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINK_OFF), eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_InitDisplay: Failed to turn off display (%x) LCD %d", command, lcd);
        
        is_init_successful = false;
    }

    if (!LCD_API_Send(lcd, LCD_CLEAR_DISPLAY, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_InitDisplay: Failed to clear display LCD %d", lcd);
        
        is_init_successful = false;
    }

    osDelay(2);

    command = (LCD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT | LCD_ENTRY_SHIFT_OFF);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_InitDisplay: Failed to set entry mode (%x) LCD %d", command, lcd);
        
        is_init_successful = false;
    }

    command = (LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_InitDisplay: Failed to turn off display (%x) LCD %d", command, lcd);
        
        is_init_successful = false;
    }

    return is_init_successful;
}

static bool LCD_API_SetCursor (const eLcd_t lcd, const eLcdRow_t row, const eLcdColumn_t column) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_SetCursor: Incorrect LCD type %d", lcd);
        
        return false;
    }

    uint8_t command = (LCD_SET_DDRAM_ADDR | (g_row_offset[row] + column));

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_SetCursor: Failed to set cursor (%d, %d) LCD %d", row, column, lcd);
        
        return false;
    }

    return true;
}

static bool LCD_API_SendBytes (const eLcd_t lcd, const char *data, const size_t data_size, const eLcdTxMode_t tx_mode) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_SendBytes: Incorrect LCD type %d", lcd);
        
        return false;
    }

    if (data == NULL || data_size == 0) {
        TRACE_ERR("LCD_API_SendBytes: Invalid data or size");
        
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

    for (eLcd_t lcd = (eLcd_First + 1); lcd < eLcd_Last; lcd++) {
        if (!I2C_API_Init(g_static_lcd_lut[lcd].i2c)) {
            TRACE_ERR("LCD_API_InitAllLcd: Failed to initialize I2C for LCD %d", lcd);

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
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, LCD_CLEAR_DISPLAY, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_Clear: Failed to clear display for LCD %d", lcd);
        
        return false;
    }

    osDelay(2);

    return true;
}

bool LCD_API_TurnOn (const eLcd_t lcd) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, (LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON), eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_Clear: Failed to turn on display for LCD %d", lcd);
        
        return false;
    }

    return true;
}

bool LCD_API_TurnOff (const eLcd_t lcd) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, (LCD_DISPLAY_CONTROL | LCD_DISPLAY_OFF), eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_Clear: Failed to turn off display for LCD %d", lcd);
        
        return false;
    }

    return true;
}

bool LCD_API_SendCommand (const eLcd_t lcd, const uint8_t command) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }

    if (!LCD_API_Send(lcd, command, eLcdTxMode_Command)) {
        TRACE_ERR("LCD_API_SendCommand: Failed to send command %x to LCD %d", command, lcd);
        
        return false;
    }

    return true;
}

bool LCD_API_Print (const eLcd_t lcd, const sMessage_t *message, const eLcdRow_t row, const eLcdColumn_t column, const eLcdOption_t option) {
    if (!LCD_API_IsCorredtLcd(lcd)) {
        TRACE_ERR ("LCD_API_Send: Incorrect LCD type %d", lcd);
        
        return false;
    }

    if (message == NULL || message->data == NULL || message->size == 0) {
        TRACE_ERR("LCD_API_Print: Invalid message data");
        
        return false;
    }

    if ((row < eLcdRow_First) || (row >= eLcdRow_Last)) {
        TRACE_ERR("LCD_API_Print: Invalid row %d", row);
        
        return false;
    }

    if ((column < eLcdColumn_First) || (column >= eLcdColumn_Last)) {
        TRACE_ERR("LCD_API_Print: Invalid column %d", column);
        
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
            TRACE_ERR("LCD_API_Print: Invalid option %d", option);
            
            return false;
        }
    }

    if (!LCD_API_SetCursor(lcd, row, column)) {
        return false;
    }

    return LCD_API_SendBytes(lcd, message->data, message->size, eLcdTxMode_Data);
}

bool LCD_API_IsCorredtLcd (const eLcd_t lcd) {
    return (lcd > eLcd_First) && (lcd < eLcd_Last);
}

#endif
