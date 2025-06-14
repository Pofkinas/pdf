/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "uart_driver.h"

#ifdef USE_UART

#include "ring_buffer.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

RingBuffer_Handle g_ring_buffer[eUartDriver_Last] = {
    #ifdef USE_UART_DEBUG
    [eUartDriver_Debug] = NULL,
    #endif

    #ifdef USE_UART_UROS_TX
    [eUartDriver_uRos] = NULL,
    #endif
};

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static uint32_t g_static_baudrate_lut[eUartBaudrate_Last] = {
    [eUartBaudrate_4800] = 4800,
    [eUartBaudrate_9600] = 9600,
    [eUartBaudrate_19200] = 19200,
    [eUartBaudrate_38400] = 38400,
    [eUartBaudrate_57600] = 57600,
    [eUartBaudrate_115200] = 115200,
    [eUartBaudrate_230400] = 320400,
    [eUartBaudrate_460800] = 460800,
    [eUartBaudrate_921600] = 921600
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sUartDesc_t g_uart_lut[eUartDriver_Last];

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void UARTx_ISRHandler (const eUartDriver_t uart);
void USART1_IRQHandler (void);
void USART2_IRQHandler (void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void UARTx_ISRHandler (const eUartDriver_t uart) {
    if ((uart <= eUartDriver_First) || (uart >= eUartDriver_Last)) {
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
    #ifdef USE_UART_UROS_TX
    UARTx_ISRHandler(UART_1);
    #endif
}

void USART2_IRQHandler (void) {
    #ifdef USE_UART_DEBUG
    UARTx_ISRHandler(UART_2);
    #endif
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

void UART_Driver_DefinePerips (const sUartDesc_t *uart_lut) {
    if (uart_lut == NULL) {
        return;
    }

    g_uart_lut = uart_lut;

    return;
}

bool UART_Driver_Init (const eUartDriver_t uart, const eUartBaudrate_t baudrate) {
    if ((uart <= eUartDriver_First) || (uart >= eUartDriver_Last)) {
        return false;
    }

    if ((baudrate < eUartBaudrate_First) || (baudrate >= eUartBaudrate_Last)) {
        return false;
    }

    LL_USART_InitTypeDef uart_init_struct = {0};

    g_uart_lut[uart].enable_clock_fp(g_uart_lut[uart].clock);

    uart_init_struct.BaudRate = g_static_baudrate_lut[baudrate];
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

bool UART_Driver_SendByte (const eUartDriver_t uart, const uint8_t data) {
    if ((uart <= eUartDriver_First) || (uart >= eUartDriver_Last)) {
        return false;
    }

    if (!LL_USART_IsEnabled(g_uart_lut[uart].periph)) {
        return false;
    }

    while (!LL_USART_IsActiveFlag_TXE(g_uart_lut[uart].periph)) {}

    LL_USART_TransmitData8(g_uart_lut[uart].periph, data);
    return true;
}

bool UART_Driver_SendBytes (const eUartDriver_t uart, uint8_t *data, const size_t size) {
    if ((uart <= eUartDriver_First) || (uart >= eUartDriver_Last)) {
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

bool UART_Driver_ReceiveByte (const eUartDriver_t uart, uint8_t *data) {
    if ((uart <= eUartDriver_First) || (uart >= eUartDriver_Last)) {
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

#endif
