#ifndef SOURCE_API_I2C_API_H_
#define SOURCE_API_I2C_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#if defined(ENABLE_I2C)
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "i2c_config.h"

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

// Works only in 7-bit addresing master mode
bool I2C_API_Init (const eI2c_t i2c);
bool I2C_API_Write (const eI2c_t i2c, const uint8_t device_address, uint8_t *data, const size_t data_size, const uint16_t mem_address, const uint8_t mem_address_size, uint32_t timeout);
bool I2C_API_Read (const eI2c_t i2c, const uint8_t device_address, uint8_t *data, const size_t data_size, const uint16_t mem_address, const uint8_t mem_address_size, uint32_t timeout);

#endif /* ENABLE_I2C */
#endif /* SOURCE_API_I2C_API_H_ */
