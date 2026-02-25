/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "uart_api.h"

#ifdef ENABLE_UART
#include "cmsis_os2.h"
#include "debug_api.h"
#include "heap_api.h"
#include "uart_driver.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eState {
    eState_First,
    eState_Uninitialized = eState_First,
    eState_Setup,
    eState_Collect,
    eState_Flush,
    eState_Last
} eState_t;

typedef struct sUartApiDynamic {
    eState_t current_state;
    osMutexId_t mutex_send;
    osMessageQueueId_t message_queue;
    sMessage_t message;
    char *delimiter;
    size_t delimiter_length;
} sUartDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_UART_API
CREATE_MODULE_NAME (UART_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_UART_API */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static osThreadId_t g_fsm_thread_id = NULL;

static sUartApiConst_t g_static_uart_lut[eUart_Last] = {0};
static sUartDynamic_t g_dynamic_uart_lut[eUart_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void UART_API_FsmThread (void *arg);
static bool UART_API_IsDelimiterReceived (const eUart_t uart);
static void UART_API_BufferIncrement (const eUart_t uart);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

// TODO: Refactor thread to use event flags or something to avoid busy waiting
// TODO: Add enum in static lut to select between message/delimiter modes, i.e. read until delimiter or include payload length
static void UART_API_FsmThread (void *arg) {
    while (1) {
        for (eUart_t uart = eUart_First; uart < eUart_Last; uart++) {
            if (g_dynamic_uart_lut[uart].current_state == eState_Uninitialized) {
                continue;
            }

            switch (g_dynamic_uart_lut[uart].current_state) {
                case eState_Setup: {
                    g_dynamic_uart_lut[uart].message.data = Heap_API_Calloc(g_static_uart_lut[uart].buffer_capacity, sizeof(char));
                    
                    if (g_dynamic_uart_lut[uart].message.data == NULL) {
                            TRACE_WRN("FsmThread: Failed to allocate buffer for UART [%d]\n", uart);
                            
                            continue;
                    }
                    
                    g_dynamic_uart_lut[uart].message.size = 0;
                    g_dynamic_uart_lut[uart].current_state = eState_Collect;
                }
                case eState_Collect: {
                    uint8_t received_byte = 0;

                    while (UART_Driver_ReceiveByte(uart, &received_byte)) {
                        g_dynamic_uart_lut[uart].message.data[g_dynamic_uart_lut[uart].message.size] = received_byte;

                        UART_API_BufferIncrement(uart);

                        if (!UART_API_IsDelimiterReceived(uart)) {
                            continue;
                        }

                        g_dynamic_uart_lut[uart].message.size -= g_dynamic_uart_lut[uart].delimiter_length;
                        g_dynamic_uart_lut[uart].message.data[g_dynamic_uart_lut[uart].message.size] = '\0';

                        g_dynamic_uart_lut[uart].current_state = eState_Flush;

                        break;
                    }

                    if (g_dynamic_uart_lut[uart].current_state != eState_Flush) {
                        continue;
                    }
                }
                case eState_Flush: {
                    if (osMessageQueuePut(g_dynamic_uart_lut[uart].message_queue, &g_dynamic_uart_lut[uart].message, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_PUT_TIMEOUT) != osOK) {
                        TRACE_ERR("FsmThread: Failed to put message in queue for UART [%d]\n", uart);
                        
                        continue;
                    }

                    g_dynamic_uart_lut[uart].current_state = eState_Setup;
                } break;
                default: {  
                } break;
            }

            osThreadYield();
        }
    }
}

static void UART_API_BufferIncrement (const eUart_t uart) {
    g_dynamic_uart_lut[uart].message.size++;

    if (g_dynamic_uart_lut[uart].message.size >= g_static_uart_lut[uart].buffer_capacity) {
        g_dynamic_uart_lut[uart].message.size = 0;
    }

    return;
}

static bool UART_API_IsDelimiterReceived (const eUart_t uart) {
    if (g_dynamic_uart_lut[uart].message.size < g_dynamic_uart_lut[uart].delimiter_length) {
        return false;
    }
    
    if (g_dynamic_uart_lut[uart].message.data[g_dynamic_uart_lut[uart].message.size - 1] != g_dynamic_uart_lut[uart].delimiter[g_dynamic_uart_lut[uart].delimiter_length - 1]) {
        return false;
    } 
    
    size_t start_pos = g_dynamic_uart_lut[uart].message.size - g_dynamic_uart_lut[uart].delimiter_length;

    return (memcmp(&g_dynamic_uart_lut[uart].message.data[start_pos], g_dynamic_uart_lut[uart].delimiter, g_dynamic_uart_lut[uart].delimiter_length) == 0);
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool UART_API_Init (const eUart_t uart, const eBaudrate_t baudrate, const char *delimiter) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return false;
    }

    if (g_dynamic_uart_lut[uart].current_state != eState_Uninitialized) {
        return true;
    }

    if ((baudrate < eBaudrate_First) || (baudrate >= eBaudrate_Last)) {
        return false;
    }

    if (delimiter == NULL) {
        return false;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    const sUartApiConst_t *api_const = UART_Config_GetUartApiConst(uart);

    if (api_const == NULL) {
        return false;
    }

    g_static_uart_lut[uart] = *api_const;
    
    if (!UART_Driver_Init(uart, baudrate)) {
        return false;
    }

    g_dynamic_uart_lut[uart].mutex_send = osMutexNew(&g_static_uart_lut[uart].mutex_send_attributes);
    
    if (g_dynamic_uart_lut[uart].mutex_send == NULL) {
        return false;
    }

    g_dynamic_uart_lut[uart].message_queue = osMessageQueueNew(MESSAGE_QUEUE_CAPACITY, sizeof(sMessage_t), &g_static_uart_lut[uart].message_queue_attributes);

    if (g_dynamic_uart_lut[uart].message_queue == NULL) {
        return false;
    }

    g_dynamic_uart_lut[uart].delimiter_length = strlen(delimiter);
    g_dynamic_uart_lut[uart].delimiter = Heap_API_Calloc((g_dynamic_uart_lut[uart].delimiter_length + 1), sizeof(char));

    if (g_dynamic_uart_lut[uart].delimiter == NULL) {
        return false;
    }

    memcpy(g_dynamic_uart_lut[uart].delimiter, delimiter, g_dynamic_uart_lut[uart].delimiter_length + 1);

    g_dynamic_uart_lut[uart].current_state = eState_Setup;

    if (g_fsm_thread_id == NULL) {
        g_fsm_thread_id = osThreadNew(UART_API_FsmThread, NULL, &g_fsm_thread_attributes);
    }

    if (g_fsm_thread_id == NULL) {
        return false;
    }

    return true;
}

// TODO: Revork UART_API_Send to use asynchronous sending with message queues and FSM thread + mutiple messages in buffer
bool UART_API_Send (const eUart_t uart, const sMessage_t message, const uint32_t timeout) {
    if (!UART_Config_IsCorrectUart(uart)) {
        TRACE_ERR("Send: Incorrect UART type [%d]\n", uart);
        
        return false;
    }

    if (g_dynamic_uart_lut[uart].current_state == eState_Uninitialized) {
        TRACE_ERR("Send: UART [%d] not initialized\n", uart);
        
        return false;
    }
    
    if (osMutexAcquire(g_dynamic_uart_lut[uart].mutex_send, timeout) != osOK) {
        TRACE_ERR("Send: Failed to acquire mutex for UART [%d]\n", uart);
        
        return false;
    }

    if (!UART_Driver_SendBytes(uart, (uint8_t*) message.data, message.size)) {
        TRACE_ERR("Send: Failed to send bytes for UART [%d]\n", uart);
        
        osMutexRelease(g_dynamic_uart_lut[uart].mutex_send);
        
        return false;
    }

    osMutexRelease(g_dynamic_uart_lut[uart].mutex_send);

    return true;
}

bool UART_API_Receive (const eUart_t uart, sMessage_t *message, const uint32_t timeout) {
    if (!UART_Config_IsCorrectUart(uart)) {
        TRACE_ERR("Receive: Incorrect UART type [%d]\n", uart);
        
        return false;
    }

    if (g_dynamic_uart_lut[uart].current_state == eState_Uninitialized) {
        TRACE_ERR("Receive: UART [%d] not initialized\n", uart);
        
        return false;
    }

    if (message == NULL) {
        TRACE_ERR("Receive: Message pointer is NULL for UART [%d]\n", uart);
        
        return false;
    }

    if (osMessageQueueGet(g_dynamic_uart_lut[uart].message_queue, message, MESSAGE_QUEUE_PRIORITY, timeout) != osOK) {
        TRACE_ERR("Receive: Failed to get message from queue for UART [%d]\n", uart);
        
        return false;
    }

    return true;
}

#endif /* ENABLE_UART */
