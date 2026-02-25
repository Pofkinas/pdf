/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "ring_buffer.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/* clang-format off */
struct sRingBufferDesc {
    size_t buffer_capacity;
    size_t head;
    size_t tail;
    size_t count;
    uint8_t *buffer;
};
/* clang-format on */

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

RingBuffer_Handle Ring_Buffer_Init (size_t buffer_capacity) {
    RingBuffer_Handle ring_buffer = malloc(sizeof(struct sRingBufferDesc));

    if (NULL == ring_buffer) {
        free(ring_buffer);
        return NULL;
    }

    ring_buffer->buffer_capacity = buffer_capacity;

    ring_buffer->buffer = malloc(buffer_capacity);

    if (NULL == ring_buffer->buffer) {
        free(ring_buffer);
        return NULL;
    }

    ring_buffer->head = 0;
    ring_buffer->tail = 0;
    ring_buffer->count = 0;

    return ring_buffer;
}

bool Ring_Buffer_DeInit (RingBuffer_Handle ring_buffer) {
    if (NULL == ring_buffer) {
        return false;
    }

    if (NULL == ring_buffer->buffer) {
        free(ring_buffer);
        return false;
    }

    free(ring_buffer->buffer);
    free(ring_buffer);
    return true;
}

bool Ring_Buffer_IsFull (RingBuffer_Handle ring_buffer) {
    if (NULL != ring_buffer) {
        return (ring_buffer->count == ring_buffer->buffer_capacity);
    }

    return false;
}

bool Ring_Buffer_IsEmpty (RingBuffer_Handle ring_buffer) {
    if (NULL != ring_buffer) {
        return (0 == ring_buffer->count);
    }

    return false;
}

bool Ring_Buffer_Push (RingBuffer_Handle ring_buffer, uint8_t data) {
    if (NULL == ring_buffer) {
        return false;
    }
    
    ring_buffer->buffer[ring_buffer->head] = data;
    ring_buffer->head++;

    if (ring_buffer->count < ring_buffer->buffer_capacity) {
        ring_buffer->count++;
    }

    if (ring_buffer->buffer_capacity == ring_buffer->head) {
        ring_buffer->head = 0;
    }

    if (Ring_Buffer_IsFull(ring_buffer)) {
        ring_buffer->tail = ring_buffer->head;
    }
    
    return true;
}

bool Ring_Buffer_Pop (RingBuffer_Handle ring_buffer, uint8_t *data) {
    if ((NULL == ring_buffer) || (NULL == data)) {
        return false;
    }

    if (Ring_Buffer_IsEmpty(ring_buffer)) {
        return false;
    }

    *data = ring_buffer->buffer[ring_buffer->tail];
    ring_buffer->tail++;

    if (ring_buffer->count > 0) {
        ring_buffer->count--;
    }

    if (ring_buffer->buffer_capacity == ring_buffer->tail) {
        ring_buffer->tail = 0;
    }

    return true;
}
