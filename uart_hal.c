#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart_hal.h"

/* Ring buffer implementation */
void ring_buf_init(ring_buf_t *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    memset(rb->buf, 0, RING_BUF_SIZE);
}

bool ring_buf_write(ring_buf_t *rb, uint8_t data) {
    if (rb->count >= RING_BUF_SIZE) {
        return false; /* Buffer full */
    }
    rb->buf[rb->head] = data;
    rb->head = (rb->head + 1) % RING_BUF_SIZE;
    rb->count++;
    return true;
}

bool ring_buf_read(ring_buf_t *rb, uint8_t *data) {
    if (rb->count == 0) {
        return false; /* Buffer empty */
    }
    *data = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUF_SIZE;
    rb->count--;
    return true;
}

/* UART HAL */
uart_err_t uart_init(uart_dev_t *dev, uint32_t baud) {
    if (!dev) return UART_ERR_NULL;
    ring_buf_init(&dev->tx_buf);
    ring_buf_init(&dev->rx_buf);
    dev->baud = baud;
    dev->initialized = true;
    dev->err_count = 0;
    printf("[UART_HAL] Initialized at %u baud\n", baud);
    return UART_OK;
}

uart_err_t uart_write(uart_dev_t *dev, const uint8_t *data, uint16_t len) {
    if (!dev || !dev->initialized) return UART_ERR_NOT_INIT;
    if (!data) return UART_ERR_NULL;
    for (uint16_t i = 0; i < len; i++) {
        if (!ring_buf_write(&dev->tx_buf, data[i])) {
            dev->err_count++;
            return UART_ERR_OVERFLOW;
        }
    }
    /* Flush TX buffer - simulate transmission */
    uint8_t byte;
    while (ring_buf_read(&dev->tx_buf, &byte)) {
        putchar(byte);
    }
    fflush(stdout);
    return UART_OK;
}

uart_err_t uart_puts(uart_dev_t *dev, const char *str) {
    return uart_write(dev, (const uint8_t*)str, strlen(str));
}

uint32_t uart_get_err_count(uart_dev_t *dev) {
    return dev ? dev->err_count : 0;
}
