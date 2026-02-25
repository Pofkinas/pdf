/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "i2c_api.h"

#if defined(ENABLE_I2C)
#include "cmsis_os2.h"
#include "debug_api.h"
#include "i2c_driver.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define I2C_FLAG_SUCCESS 0x01U
#define I2C_FLAG_ERROR 0x02U
#define I2C_FLAG_BUS_RESET 0x04U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eI2cState {
    eI2cState_First = 0,
    eI2cState_Off = eI2cState_First,
    eI2cState_Idle,
    eI2cState_StartComms,
    eI2cState_SendMemAddress,
    eI2cState_RestartComms,
    eI2cState_SendData,
    eI2cState_PrepRead,
    eI2cState_ReadData,
    eI2cState_Success,
    eI2cState_Error,
    eI2cState_Last
} eI2cState_t;

typedef struct sI2cDynamicDesc {
    eI2c_t i2c;
    eI2cState_t state;
    eI2cState_t previous_state;
    uint8_t rw_operation;
    osEventFlagsId_t flag;
    osMutexId_t mutex;
    uint8_t device_address;
    uint8_t data[I2C_MAX_DATA_SIZE];
    size_t processed_data;
    size_t data_size;
    uint16_t mem_address;
    uint8_t mem_address_size;
} sI2cDynamicDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#if defined(DEBUG_I2C_API)
CREATE_MODULE_NAME (I2C_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_I2C_API */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sI2cOsDesc_t g_static_i2c_lut[eI2c_Last] = {0};
static sI2cDynamicDesc_t g_dynamic_i2c[eI2c_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void I2C_API_IrsCallback (const eI2c_Flags_t flag, void *context);
#if defined(ENABLE_UART_DEBUG)
static char *I2C_API_GetStateString (const eI2cState_t state);
#endif /* ENABLE_UART_DEBUG */
static bool I2C_API_StartComms (const eI2c_t i2c);
static bool I2C_API_SendMemAddress (const eI2c_t i2c);
static bool I2C_API_SendData (const eI2c_t i2c);
static bool I2C_API_PrepRead (const eI2c_t i2c);
static bool I2C_API_ReadData (const eI2c_t i2c);
static void I2C_API_HandleSuccess (const eI2c_t i2c);
static void I2C_API_HandleError (const eI2c_t i2c);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void I2C_API_IrsCallback (const eI2c_Flags_t flag, void *context) {
    if (NULL == context) {
        return;
    }

    sI2cDynamicDesc_t *i2c = (sI2cDynamicDesc_t*) context;

    bool (*opperation) (const eI2c_t i2c) = NULL;

    switch (i2c->state) {
        case eI2cState_StartComms: {
            if (eI2c_Flags_Addr == flag) {
                I2C_Driver_ClearFlag(i2c->i2c, flag);

                i2c->state = eI2cState_SendMemAddress;

                opperation = I2C_API_SendMemAddress;
            }
        } break;
        case eI2cState_SendMemAddress: {
            if (eI2c_Flags_Txe == flag) {
                if (i2c->mem_address_size > 0) {
                    opperation = I2C_API_SendMemAddress;

                    break;
                }
                
                if (I2C_WRITE == i2c->rw_operation) {
                    i2c->state = eI2cState_SendData;

                    opperation = I2C_API_SendData;
                } else if (I2C_READ == i2c->rw_operation) {
                    i2c->state = eI2cState_RestartComms;

                    opperation = I2C_API_StartComms;
                }
            }
        } break;
        case eI2cState_RestartComms: {
            if (eI2c_Flags_Addr == flag) {
                i2c->state = eI2cState_PrepRead;

                opperation = I2C_API_PrepRead;
            }
        } break;
        case eI2cState_SendData: {
            if (eI2c_Flags_Txe == flag) {
                opperation = I2C_API_SendData;
            }
        } break;
        case eI2cState_PrepRead: {
            if (eI2c_Flags_Rxne == flag) {
                i2c->state = eI2cState_ReadData;

                opperation = I2C_API_ReadData;
            }
        } break;
        case eI2cState_ReadData: {
            if (eI2c_Flags_Rxne == flag) {
                opperation = I2C_API_ReadData;
            }
        } break;
        case eI2cState_Error: {
            if (eI2c_Flags_BusReset == flag) {
                osEventFlagsSet(i2c->flag, I2C_FLAG_BUS_RESET);

                return;
            }
            /* fall through */
        }
        default: {
        } break;
    }

    if (NULL != opperation) {
        if (!opperation(i2c->i2c)) {
            TRACE_ERR("IrsCallback: Invalid FSM State [%s]\n", I2C_API_GetStateString(i2c->state));

            i2c->state = eI2cState_Error;

            I2C_API_HandleError(i2c->i2c);
        }
    }

    return;
}

#if defined(ENABLE_UART_DEBUG)
static char *I2C_API_GetStateString (const eI2cState_t state) {
    char *state_string = NULL;
    
    switch (state) {
        case eI2cState_StartComms: {
            state_string = "Start Comms";
        } break;
        case eI2cState_SendMemAddress: {
            state_string = "Send Memory Address";
        } break;
        case eI2cState_RestartComms: {
            state_string = "Restart Comms";
        } break;
        case eI2cState_SendData: {
            state_string = "Send Data";
        } break;
        case eI2cState_PrepRead: {
            state_string = "Prepare Read";
        } break;
        case eI2cState_ReadData: {
            state_string = "Read Data";
        } break;
        default: {
            state_string = "Unknown State";
        } break;
    }

    return state_string;
}
#endif /* ENABLE_UART_DEBUG */

static bool I2C_API_StartComms (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }

    uint8_t opperation = 0;

    switch (g_dynamic_i2c[i2c].state) {
        case eI2cState_StartComms: {
            opperation = I2C_WRITE;
        } break;
        case eI2cState_RestartComms: {
            opperation = I2C_READ;
        } break;
        default: {
            return false;
        } break;
    }

    if (!I2C_Driver_StartComms(i2c, g_dynamic_i2c[i2c].device_address, opperation)) {
        return false;
    }

    return true;
}

static bool I2C_API_SendMemAddress (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }

    if (eI2cState_SendMemAddress != g_dynamic_i2c[i2c].state) {
        return false;
    }

    if (0 == g_dynamic_i2c[i2c].data_size) {
        g_dynamic_i2c[i2c].state = eI2cState_Success;

        I2C_Driver_StopComms(i2c);

        I2C_API_HandleSuccess(i2c);

        return true;
    }

    if (0 != g_dynamic_i2c[i2c].mem_address_size) {
        switch (g_dynamic_i2c[i2c].mem_address_size) {
            case 1: {
                I2C_Driver_SendByte(i2c, (uint8_t) (g_dynamic_i2c[i2c].mem_address & 0xFF));
            } break;
            case 2: {
                I2C_Driver_SendByte(i2c, (uint8_t) (g_dynamic_i2c[i2c].mem_address >> 8));
            } break;
            default: {
                return false;
            }    
        }
        g_dynamic_i2c[i2c].mem_address_size--;
    }

    return true;
}

static bool I2C_API_SendData (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }
    
    if (eI2cState_SendData != g_dynamic_i2c[i2c].state) {
        return false;
    }

    if (g_dynamic_i2c[i2c].processed_data >= g_dynamic_i2c[i2c].data_size) {
        if (!I2C_Driver_StopComms(i2c)) {
            return false;
        }

        g_dynamic_i2c[i2c].state = eI2cState_Success;

        I2C_API_HandleSuccess(i2c);

        return true;
    }

    if (!I2C_Driver_SendByte(i2c, g_dynamic_i2c[i2c].data[g_dynamic_i2c[i2c].processed_data])) {
        return false;
    }

    g_dynamic_i2c[i2c].processed_data++;

    return true;
}

static bool I2C_API_PrepRead (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }

    if (eI2cState_PrepRead != g_dynamic_i2c[i2c].state) {
        return false;
    }

    if (1 == g_dynamic_i2c[i2c].data_size) {
        if (!I2C_Driver_Acknowledge(i2c, false)) {
            return false;
        }

        I2C_Driver_ClearFlag(i2c, eI2c_Flags_Addr);

        if (!I2C_Driver_StopComms(i2c)) {
            return false;
        }
    } else {
        I2C_Driver_ClearFlag(i2c, eI2c_Flags_Addr);

        if (!I2C_Driver_Acknowledge(i2c, true)) {
            return false;
        }
    }

    return true;
}

static bool I2C_API_ReadData (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }

    if (0 == g_dynamic_i2c[i2c].data_size) {
        return false;
    }

    if (0 == g_dynamic_i2c[i2c].data_size) {
        return false;
    }

    switch (g_dynamic_i2c[i2c].data_size) {
        case 1: {
            if (!I2C_Driver_ReadByte(i2c, &g_dynamic_i2c[i2c].data[g_dynamic_i2c[i2c].processed_data])) {
                return false;
            }
        } break;
        case 2: {
            if (!I2C_Driver_ReadByteAck(i2c, &g_dynamic_i2c[i2c].data[g_dynamic_i2c[i2c].processed_data], false)) {
                return false;
            }

            if (!I2C_Driver_StopComms(i2c)) {
                return false;
            }
        } break;
        default: {
            if (!I2C_Driver_ReadByteAck(i2c, &g_dynamic_i2c[i2c].data[g_dynamic_i2c[i2c].processed_data], true)) {
                return false;
            }
        } break;
    }

    g_dynamic_i2c[i2c].data_size--;
    g_dynamic_i2c[i2c].processed_data++;

    if (0 == g_dynamic_i2c[i2c].data_size) {
        g_dynamic_i2c[i2c].state = eI2cState_Success;

        I2C_API_HandleSuccess(i2c);
    }

    return true;
}

static void I2C_API_HandleSuccess (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return;
    }

    if (eI2cState_Success != g_dynamic_i2c[i2c].state) {
        return;
    }

    I2C_Driver_DisableIt(i2c);

    while (I2C_Driver_CheckFlag(i2c, eI2c_Flags_Busy)) {}

    osEventFlagsSet(g_dynamic_i2c[i2c].flag, I2C_FLAG_SUCCESS);

    return;
}

static void I2C_API_HandleError (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return;
    }

    if (eI2cState_Error != g_dynamic_i2c[i2c].state) {
        return;
    }

    I2C_Driver_DisableIt(i2c);

    I2C_Driver_ResetLine(i2c);

    osEventFlagsSet(g_dynamic_i2c[i2c].flag, I2C_FLAG_ERROR);

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool I2C_API_Init (const eI2c_t i2c) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }

    if (eI2cState_Off != g_dynamic_i2c[i2c].state) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    if (!I2C_Driver_Init(i2c, &I2C_API_IrsCallback, &g_dynamic_i2c[i2c])) {
        return false;
    }

    const sI2cOsDesc_t *desc = I2C_Config_GetI2cOsDesc(i2c);

    if (NULL == desc) {
        return false;
    }

    g_static_i2c_lut[i2c] = *desc;

    g_dynamic_i2c[i2c].flag = osEventFlagsNew(&g_static_i2c_lut[i2c].flag_attributes);
    
    if (NULL == g_dynamic_i2c[i2c].flag) {
        return false;
    }

    g_dynamic_i2c[i2c].mutex = osMutexNew(&g_static_i2c_lut[i2c].mutex_attributes);

    if (NULL == g_dynamic_i2c[i2c].mutex) {
        return false;
    }

    g_dynamic_i2c[i2c].i2c = i2c;
    g_dynamic_i2c[i2c].state = eI2cState_Idle;

    return true;
}

bool I2C_API_Write (const eI2c_t i2c, const uint8_t device_address, uint8_t *data, const size_t data_size, const uint16_t mem_address, const uint8_t mem_address_size, const uint32_t timeout) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }
    
    if ((0 == timeout) || (data_size > I2C_MAX_DATA_SIZE)) {
        return false;
    }

    if (eI2cState_Idle != g_dynamic_i2c[i2c].state) {
        return false;
    }

    if (I2C_Driver_CheckFlag(i2c, eI2c_Flags_Busy)) {
        TRACE_ERR("Write: I2C is busy\n");

        return false;
    }

    // TODO: Add RTOS-independent I2C support

    if (osOK != osMutexAcquire(g_dynamic_i2c[i2c].mutex, MUTEX_TIMEOUT)) {
        return false;
    }

    memcpy(g_dynamic_i2c[i2c].data, data, data_size);

    g_dynamic_i2c[i2c].rw_operation = I2C_WRITE;
    g_dynamic_i2c[i2c].device_address = device_address;
    g_dynamic_i2c[i2c].processed_data = 0;
    g_dynamic_i2c[i2c].data_size = data_size;
    g_dynamic_i2c[i2c].mem_address = mem_address;
    g_dynamic_i2c[i2c].mem_address_size = mem_address_size;

    g_dynamic_i2c[i2c].state = eI2cState_StartComms;

    osMutexRelease(g_dynamic_i2c[i2c].mutex);

    I2C_Driver_EnableIt(i2c);
    I2C_API_StartComms(i2c);

    uint32_t flag = osEventFlagsWait(g_dynamic_i2c[i2c].flag, I2C_FLAG_SUCCESS | I2C_FLAG_ERROR, osFlagsWaitAny, timeout);

    if (osOK != osMutexAcquire(g_dynamic_i2c[i2c].mutex, MUTEX_TIMEOUT)) {
        return false;
    }

    if ((int32_t) flag < 0) {
        g_dynamic_i2c[i2c].previous_state = g_dynamic_i2c[i2c].state;
        g_dynamic_i2c[i2c].state = eI2cState_Error;
    } else {
        g_dynamic_i2c[i2c].state = eI2cState_Idle;
    }

    osMutexRelease(g_dynamic_i2c[i2c].mutex);

    if (eI2cState_Error == g_dynamic_i2c[i2c].state) {
        I2C_Driver_DisableIt(i2c);

        TRACE_ERR("Write: Error event flag [%ld], I2C state: [%s]\n", (int32_t) flag, I2C_API_GetStateString(g_dynamic_i2c[i2c].previous_state));

        I2C_API_HandleError(i2c);

        flag = osEventFlagsWait(g_dynamic_i2c[i2c].flag, I2C_FLAG_BUS_RESET, osFlagsWaitAny, BUS_RESET_TIMEOUT);

        if ((I2C_FLAG_BUS_RESET | I2C_FLAG_ERROR) != flag) {
            TRACE_ERR("Read: Failed reset bus, received flag [%ld]\n", (int32_t) flag);
        }

        osEventFlagsClear(g_dynamic_i2c[i2c].flag, I2C_FLAG_ERROR);

        if (osOK != osMutexAcquire(g_dynamic_i2c[i2c].mutex, MUTEX_TIMEOUT)) {
            return false;
        }

        g_dynamic_i2c[i2c].state = eI2cState_Idle;

        osMutexRelease(g_dynamic_i2c[i2c].mutex);

        return false;
    }

    return (I2C_FLAG_SUCCESS == flag);
}

bool I2C_API_Read (const eI2c_t i2c, const uint8_t device_address, uint8_t *data, const size_t data_size, const uint16_t mem_address, const uint8_t mem_address_size, uint32_t timeout) {
    if (!I2C_Config_IsCorrectI2c(i2c)) {
        return false;
    }
    
    if ((NULL == data) || (0 == data_size) || (data_size > I2C_MAX_DATA_SIZE)) {
        return false;
    }

    if (0 == timeout) {
        return false;
    }

    if (eI2cState_Idle != g_dynamic_i2c[i2c].state) {
        return false;
    }

    if (I2C_Driver_CheckFlag(i2c, eI2c_Flags_Busy)) {
        TRACE_ERR("Read: I2C is busy\n");

        return false;
    }

    if (osOK != osMutexAcquire(g_dynamic_i2c[i2c].mutex, MUTEX_TIMEOUT)) {
        return false;
    }

    memset(g_dynamic_i2c[i2c].data, 0, data_size);

    g_dynamic_i2c[i2c].rw_operation = I2C_READ;
    g_dynamic_i2c[i2c].device_address = device_address;
    g_dynamic_i2c[i2c].processed_data = 0;
    g_dynamic_i2c[i2c].data_size = data_size;
    g_dynamic_i2c[i2c].mem_address = mem_address;
    g_dynamic_i2c[i2c].mem_address_size = mem_address_size;

    g_dynamic_i2c[i2c].state = eI2cState_StartComms;

    osMutexRelease(g_dynamic_i2c[i2c].mutex);

    I2C_Driver_EnableIt(i2c);
    I2C_API_StartComms(i2c);

    uint32_t flag = osEventFlagsWait(g_dynamic_i2c[i2c].flag, I2C_FLAG_SUCCESS | I2C_FLAG_ERROR, osFlagsWaitAny, timeout);

    if (osOK != osMutexAcquire(g_dynamic_i2c[i2c].mutex, MUTEX_TIMEOUT)) {
        return false;
    }

    if ((int32_t) flag < 0) {
        g_dynamic_i2c[i2c].previous_state = g_dynamic_i2c[i2c].state;
        g_dynamic_i2c[i2c].state = eI2cState_Error;
    } else {
        g_dynamic_i2c[i2c].state = eI2cState_Idle;
    }

    if (eI2cState_Error == g_dynamic_i2c[i2c].state) {
        osMutexRelease(g_dynamic_i2c[i2c].mutex);
        
        I2C_Driver_DisableIt(i2c);

        TRACE_ERR("Read: Error event flag [%ld], I2C state: [%s]\n", (int32_t) flag, I2C_API_GetStateString(g_dynamic_i2c[i2c].previous_state));

        I2C_API_HandleError(i2c);

        flag = osEventFlagsWait(g_dynamic_i2c[i2c].flag, I2C_FLAG_BUS_RESET, osFlagsWaitAny, BUS_RESET_TIMEOUT);
        
        if ((I2C_FLAG_BUS_RESET | I2C_FLAG_ERROR) != flag) {
            TRACE_ERR("Read: Failed reset bus, received flag [%ld]\n", (int32_t) flag);
        }

        osEventFlagsClear(g_dynamic_i2c[i2c].flag, I2C_FLAG_ERROR);

        if (osOK != osMutexAcquire(g_dynamic_i2c[i2c].mutex, MUTEX_TIMEOUT)) {
            return false;
        }

        g_dynamic_i2c[i2c].state = eI2cState_Idle;

        osMutexRelease(g_dynamic_i2c[i2c].mutex);

        return false;
    }

    if (I2C_FLAG_SUCCESS == flag) {
        memcpy(data, g_dynamic_i2c[i2c].data, data_size);
    }

    osMutexRelease(g_dynamic_i2c[i2c].mutex);

    return (I2C_FLAG_SUCCESS == flag);
}

#endif /* ENABLE_I2C */
