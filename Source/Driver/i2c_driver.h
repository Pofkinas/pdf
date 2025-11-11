#ifndef SOURCE_DRIVER_I2C_DRIVER_H_
#define SOURCE_DRIVER_I2C_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_I2C
#include <stdbool.h>
#include <stdint.h>
#include "i2c_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eI2c_Flags {
    eI2c_Flags_First = 0,
    eI2c_Flags_Busy = eI2c_Flags_First,
    eI2c_Flags_StartBit,
    eI2c_Flags_Addr,
    eI2c_Flags_Txe,
    eI2c_Flags_ByteTransferFinish,
    eI2c_Flags_Rxne,
    eI2c_Flags_AckFailure,
    eI2c_Flags_BusError,
    eI2c_Flags_BusReset,
    eI2c_Flags_Last
} eI2c_Flags_t;

typedef void (*i2c_callback_t) (const eI2c_Flags_t flag, void *context);

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool I2C_Driver_Init (const eI2c_t i2c, i2c_callback_t flag_callback, void *context);
bool I2C_Driver_EnableIt (const eI2c_t i2c);
bool I2C_Driver_DisableIt (const eI2c_t i2c);
bool I2C_Driver_StartComms (const eI2c_t i2c, const uint8_t address, const uint8_t rw_operation);
bool I2C_Driver_StopComms (const eI2c_t i2c);
bool I2C_Driver_Acknowledge (const eI2c_t i2c, const bool ack);
bool I2C_Driver_SendByte (const eI2c_t i2c, const uint8_t data);
bool I2C_Driver_ReadByte (const eI2c_t i2c, uint8_t *data);
bool I2C_Driver_ReadByteAck (const eI2c_t i2c, uint8_t *data, const bool ack);
bool I2C_Driver_CheckFlag (const eI2c_t i2c, const eI2c_Flags_t flag);
void I2C_Driver_ClearFlag (const eI2c_t i2c, const eI2c_Flags_t flag);
bool I2C_Driver_ResetLine (const eI2c_t i2c);

#endif /* ENABLE_I2C */
#endif /* SOURCE_DRIVER_I2C_DRIVER_H_ */
