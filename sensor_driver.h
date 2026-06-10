#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define SENSOR_MAX_RETRIES 3
#define SENSOR_TIMEOUT_MS  500

typedef enum {
    SENSOR_STATE_INIT = 0,
    SENSOR_STATE_READ,
    SENSOR_STATE_VALIDATE,
    SENSOR_STATE_ERROR,
    SENSOR_STATE_RETRY
} sensor_state_t;

typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERR_TIMEOUT,
    SENSOR_ERR_CRC,
    SENSOR_ERR_RANGE,
    SENSOR_ERR_I2C,
    SENSOR_ERR_MAX_RETRY
} sensor_err_t;

typedef struct {
    float        temperature;
    float        humidity;
    bool         valid;
    uint32_t     read_count;
    uint32_t     error_count;
    sensor_state_t state;
    uint8_t      retry_count;
} sensor_data_t;

sensor_err_t sensor_init(sensor_data_t *s);
sensor_err_t sensor_read(sensor_data_t *s);
const char*  sensor_state_str(sensor_state_t state);
const char*  sensor_err_str(sensor_err_t err);

#endif
