#ifndef SOURCE_DRIVER_I2C_DRIVER_H_
#define SOURCE_DRIVER_I2C_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "framework_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define I2C_WRITE 0
#define I2C_READ  1

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eI2cDriver {
    eI2cDriver_First = 0,
    
    #ifdef USE_I2C1
    eI2cDriver_1,
    #endif

    eI2cDriver_Last
} eI2cDriver_t;

typedef enum eI2cDriver_Flags {
    eI2cDriver_Flags_First = 0,
    eI2cDriver_Flags_Busy = eI2cDriver_Flags_First,
    eI2cDriver_Flags_StartBit,
    eI2cDriver_Flags_Addr,
    eI2cDriver_Flags_Txe,
    eI2cDriver_Flags_ByteTransferFinish,
    eI2cDriver_Flags_Rxne,
    eI2cDriver_Flags_AckFailure,
    eI2cDriver_Flags_BusError,
    eI2cDriver_Flags_BusReset,
    eI2cDriver_Flags_Last
} eI2cDriver_Flags_t;
/* clang-format on */

typedef void (*i2c_callback_t) (const eI2cDriver_Flags_t flag, void *context);

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

void I2C_Driver_DefinePerips (const sI2cDesc_t *i2c_lut);
bool I2C_Driver_Init (const eI2cDriver_t i2c, i2c_callback_t flag_callback, void *context);
bool I2C_Driver_EnableIt (const eI2cDriver_t i2c);
bool I2C_Driver_DisableIt (const eI2cDriver_t i2c);
bool I2C_Driver_StartComms (const eI2cDriver_t i2c, const uint8_t address, const uint8_t rw_operation);
bool I2C_Driver_StopComms (const eI2cDriver_t i2c);
bool I2C_Driver_Acknowledge (const eI2cDriver_t i2c, const bool ack);
bool I2C_Driver_SendByte (const eI2cDriver_t i2c, const uint8_t data);
bool I2C_Driver_ReadByte (const eI2cDriver_t i2c, uint8_t *data);
bool I2C_Driver_ReadByteAck (const eI2cDriver_t i2c, uint8_t *data, const bool ack);
bool I2C_Driver_CheckFlag (const eI2cDriver_t i2c, const eI2cDriver_Flags_t flag);
void I2C_Driver_ClearFlag (const eI2cDriver_t i2c, const eI2cDriver_Flags_t flag);
bool I2C_Driver_ResetLine (const eI2cDriver_t i2c);

#endif /* SOURCE_DRIVER_I2C_DRIVER_H_ */
