/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "uart_driver.h"

#ifdef ENABLE_UART
#include "ring_buffer.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sUartDesc_t g_uart_lut[eUart_Last] = {0};
static RingBuffer_Handle g_ring_buffer[eUart_Last] = {NULL};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void UARTx_ISRHandler (const eUart_t uart);
void USART1_IRQHandler (void);
void USART2_IRQHandler (void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

 static void UARTx_ISRHandler (const eUart_t uart) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return;
    }

    if (!LL_USART_IsEnabled(g_uart_lut[uart].periph)) {
        return;
    }
    
    if (!LL_USART_IsActiveFlag_RXNE(g_uart_lut[uart].periph)) {
        return;
    }
    
    Ring_Buffer_Push(g_ring_buffer[uart], LL_USART_ReceiveData8(g_uart_lut[uart].periph));
    
    return;
}

void USART1_IRQHandler (void) {
    #ifdef UART1
    UARTx_ISRHandler(UART1);
    #endif /* UART1 */

    return;
}

void USART2_IRQHandler (void) {
    #ifdef UART2
    UARTx_ISRHandler(UART2);
    #endif /* UART2 */

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool UART_Driver_Init (const eUart_t uart, const eBaudrate_t baudrate) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return false;
    }

    if ((baudrate < eBaudrate_First) || (baudrate >= eBaudrate_Last)) {
        return false;
    }

    LL_USART_InitTypeDef uart_init_struct = {0};

    const sUartDesc_t *uart_desc = UART_Config_GetUartDesc(uart);

    if (uart_desc == NULL) {
        return false;
    }

    g_uart_lut[uart] = *uart_desc;

    g_uart_lut[uart].enable_clock_fp(g_uart_lut[uart].clock);

    uart_init_struct.BaudRate = (baudrate == eBaudrate_Default) ? Baudrate_GetValue(g_uart_lut[uart].baud) : Baudrate_GetValue(baudrate);
    uart_init_struct.DataWidth = g_uart_lut[uart].data_bits;
    uart_init_struct.StopBits = g_uart_lut[uart].stop_bits;
    uart_init_struct.Parity = g_uart_lut[uart].parity;
    uart_init_struct.TransferDirection = g_uart_lut[uart].direction;
    uart_init_struct.HardwareFlowControl = g_uart_lut[uart].flow_control;
    uart_init_struct.OverSampling = g_uart_lut[uart].oversample;

    if (LL_USART_Init(g_uart_lut[uart].periph, &uart_init_struct) == ERROR) {
        return false;
    }

    LL_USART_ConfigAsyncMode(g_uart_lut[uart].periph);

    NVIC_EnableIRQ(g_uart_lut[uart].nvic);

    if (g_uart_lut[uart].direction == LL_USART_DIRECTION_RX || g_uart_lut[uart].direction == LL_USART_DIRECTION_TX_RX) {
        LL_USART_EnableIT_RXNE(g_uart_lut[uart].periph);

        g_ring_buffer[uart] = Ring_Buffer_Init(g_uart_lut[uart].ring_buffer_capacity);

        if (g_ring_buffer[uart] == NULL) {
            return false;
        }
    }

    LL_USART_Enable(g_uart_lut[uart].periph);

    return true;
}

bool UART_Driver_SendByte (const eUart_t uart, const uint8_t data) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return false;
    }

    if (!LL_USART_IsEnabled(g_uart_lut[uart].periph)) {
        return false;
    }

    while (!LL_USART_IsActiveFlag_TXE(g_uart_lut[uart].periph)) {}

    LL_USART_TransmitData8(g_uart_lut[uart].periph, data);
    return true;
}

bool UART_Driver_SendBytes (const eUart_t uart, uint8_t *data, const size_t size) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return false;
    }

    if ((data == NULL) || (size == 0)) {
        return false;
    }

    for (size_t i = 0; i < size; i++) {
        if (!UART_Driver_SendByte(uart, data[i])) {
            return false;
        }
    }

    return true;
}

bool UART_Driver_ReceiveByte (const eUart_t uart, uint8_t *data) {
    if (!UART_Config_IsCorrectUart(uart)) {
        return false;
    }

    if (!LL_USART_IsEnabled(g_uart_lut[uart].periph)) {
        return false;
    }

    if (data == NULL) {
        return false;
    }

    return Ring_Buffer_Pop(g_ring_buffer[uart], data);
}

#endif /* ENABLE_UART */
