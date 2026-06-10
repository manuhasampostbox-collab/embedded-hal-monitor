#ifndef UART_HAL_H
#define UART_HAL_H

#include <stdint.h>
#include <stdbool.h>

#define RING_BUF_SIZE 256

typedef enum {
    UART_OK = 0,
    UART_ERR_NULL,
    UART_ERR_NOT_INIT,
    UART_ERR_OVERFLOW,
    UART_ERR_TIMEOUT
} uart_err_t;

typedef struct {
    uint8_t  buf[RING_BUF_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} ring_buf_t;

typedef struct {
    ring_buf_t tx_buf;
    ring_buf_t rx_buf;
    uint32_t   baud;
    bool       initialized;
    uint32_t   err_count;
} uart_dev_t;

void      ring_buf_init(ring_buf_t *rb);
bool      ring_buf_write(ring_buf_t *rb, uint8_t data);
bool      ring_buf_read(ring_buf_t *rb, uint8_t *data);

uart_err_t uart_init(uart_dev_t *dev, uint32_t baud);
uart_err_t uart_write(uart_dev_t *dev, const uint8_t *data, uint16_t len);
uart_err_t uart_puts(uart_dev_t *dev, const char *str);
uint32_t   uart_get_err_count(uart_dev_t *dev);

#endif
