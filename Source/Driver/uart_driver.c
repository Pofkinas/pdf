/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "uart_driver.h"

#ifdef USE_UART

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_usart.h"
#include "ring_buffer.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sUartDesc {
    USART_TypeDef *periph;
    uint32_t baud;
    uint32_t data_bits;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t direction;
    uint32_t flow_control;
    uint32_t oversample;
    uint32_t clock;
    void (*enable_clock_fp) (uint32_t);
    IRQn_Type nvic;
    size_t ring_buffer_capacity;
} sUartDesc_t;

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
const static sUartDesc_t g_static_uart_lut[eUartDriver_Last] = {
    #ifdef USE_UART_DEBUG
    [eUartDriver_Debug] = {
        .periph = USART2,
        .baud = 115200,
        .data_bits = LL_USART_DATAWIDTH_8B,
        .stop_bits = LL_USART_STOPBITS_1,
        .parity = LL_USART_PARITY_NONE,
        .direction = LL_USART_DIRECTION_TX_RX,
        .flow_control = LL_USART_HWCONTROL_NONE,
        .oversample = LL_USART_OVERSAMPLING_16,
        .clock = LL_APB1_GRP1_PERIPH_USART2,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .nvic = USART2_IRQn,
        .ring_buffer_capacity = UART_DEBUG_BUFFER_CAPACITY
    }
    #endif

    #ifdef USE_UART_UROS_TX
    [eUartDriver_uRos] = {
        .periph = USART1,
        .baud = 115200,
        .data_bits = LL_USART_DATAWIDTH_8B,
        .stop_bits = LL_USART_STOPBITS_1,
        .parity = LL_USART_PARITY_NONE,
        .direction = LL_USART_DIRECTION_TX,
        .flow_control = LL_USART_HWCONTROL_NONE,
        .oversample = LL_USART_OVERSAMPLING_16,
        .clock = LL_APB2_GRP1_PERIPH_USART1,
        .enable_clock_fp = LL_APB2_GRP1_EnableClock,
        .nvic = USART1_IRQn,
    }
    #endif
};

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

    if (!LL_USART_IsEnabled(g_static_uart_lut[uart].periph)) {
        return;
    }
    
    if (!LL_USART_IsActiveFlag_RXNE(g_static_uart_lut[uart].periph)) {
        return;
    }
    
    Ring_Buffer_Push(g_ring_buffer[uart], LL_USART_ReceiveData8(g_static_uart_lut[uart].periph));
    
    return;
}

void USART1_IRQHandler (void) {
    #ifdef USE_UART_UROS_TX
    UARTx_ISRHandler(eUartDriver_uRos);
    #endif
}

void USART2_IRQHandler (void) {
    #ifdef USE_UART_DEBUG
    UARTx_ISRHandler(eUartDriver_Debug);
    #endif
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool UART_Driver_Init (const eUartDriver_t uart, const eUartBaudrate_t baudrate) {
    if ((uart <= eUartDriver_First) || (uart >= eUartDriver_Last)) {
        return false;
    }

    if ((baudrate < eUartBaudrate_First) || (baudrate >= eUartBaudrate_Last)) {
        return false;
    }

    LL_USART_InitTypeDef uart_init_struct = {0};

    g_static_uart_lut[uart].enable_clock_fp(g_static_uart_lut[uart].clock);

    uart_init_struct.BaudRate = g_static_baudrate_lut[baudrate];
    uart_init_struct.DataWidth = g_static_uart_lut[uart].data_bits;
    uart_init_struct.StopBits = g_static_uart_lut[uart].stop_bits;
    uart_init_struct.Parity = g_static_uart_lut[uart].parity;
    uart_init_struct.TransferDirection = g_static_uart_lut[uart].direction;
    uart_init_struct.HardwareFlowControl = g_static_uart_lut[uart].flow_control;
    uart_init_struct.OverSampling = g_static_uart_lut[uart].oversample;

    if (LL_USART_Init(g_static_uart_lut[uart].periph, &uart_init_struct) == ERROR) {
        return false;
    }

    LL_USART_ConfigAsyncMode(g_static_uart_lut[uart].periph);

    NVIC_EnableIRQ(g_static_uart_lut[uart].nvic);

    if (g_static_uart_lut[uart].direction == LL_USART_DIRECTION_RX || g_static_uart_lut[uart].direction == LL_USART_DIRECTION_TX_RX) {
        LL_USART_EnableIT_RXNE(g_static_uart_lut[uart].periph);

        g_ring_buffer[uart] = Ring_Buffer_Init(g_static_uart_lut[uart].ring_buffer_capacity);

        if (g_ring_buffer[uart] == NULL) {
            return false;
        }
    }

    LL_USART_Enable(g_static_uart_lut[uart].periph);

    return true;
}

bool UART_Driver_SendByte (const eUartDriver_t uart, const uint8_t data) {
    if ((uart <= eUartDriver_First) || (uart >= eUartDriver_Last)) {
        return false;
    }

    if (!LL_USART_IsEnabled(g_static_uart_lut[uart].periph)) {
        return false;
    }

    while (!LL_USART_IsActiveFlag_TXE(g_static_uart_lut[uart].periph)) {}

    LL_USART_TransmitData8(g_static_uart_lut[uart].periph, data);
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

    if (!LL_USART_IsEnabled(g_static_uart_lut[uart].periph)) {
        return false;
    }

    if (data == NULL) {
        return false;
    }

    return Ring_Buffer_Pop(g_ring_buffer[uart], data);
}

#endif
