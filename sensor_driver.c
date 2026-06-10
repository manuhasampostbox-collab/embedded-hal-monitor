#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "sensor_driver.h"

/* Simulate I2C read with occasional errors */
static sensor_err_t i2c_read_sensor(float *temp, float *hum) {
    static int call_count = 0;
    call_count++;
    /* Simulate I2C error every 7th read for error handling demo */
    if (call_count % 7 == 0) {
        return SENSOR_ERR_I2C;
    }
    /* Simulate CRC error every 11th read */
    if (call_count % 11 == 0) {
        return SENSOR_ERR_CRC;
    }
    /* Normal read - simulate realistic sensor values */
    *temp = 24.0f + ((float)(call_count % 10) * 0.5f);
    *hum  = 55.0f + ((float)(call_count % 8) * 1.0f);
    return SENSOR_OK;
}

static bool validate_range(float temp, float hum) {
    /* DHT22 valid range: temp -40 to 80C, hum 0 to 100% */
    if (temp < -40.0f || temp > 80.0f) return false;
    if (hum  <   0.0f || hum  > 100.0f) return false;
    return true;
}

const char* sensor_state_str(sensor_state_t state) {
    switch(state) {
        case SENSOR_STATE_INIT:     return "INIT";
        case SENSOR_STATE_READ:     return "READ";
        case SENSOR_STATE_VALIDATE: return "VALIDATE";
        case SENSOR_STATE_ERROR:    return "ERROR";
        case SENSOR_STATE_RETRY:    return "RETRY";
        default:                    return "UNKNOWN";
    }
}

const char* sensor_err_str(sensor_err_t err) {
    switch(err) {
        case SENSOR_OK:            return "OK";
        case SENSOR_ERR_TIMEOUT:   return "TIMEOUT";
        case SENSOR_ERR_CRC:       return "CRC_ERROR";
        case SENSOR_ERR_RANGE:     return "OUT_OF_RANGE";
        case SENSOR_ERR_I2C:       return "I2C_ERROR";
        case SENSOR_ERR_MAX_RETRY: return "MAX_RETRY_EXCEEDED";
        default:                   return "UNKNOWN_ERROR";
    }
}

sensor_err_t sensor_init(sensor_data_t *s) {
    if (!s) return SENSOR_ERR_I2C;
    s->temperature = 0.0f;
    s->humidity    = 0.0f;
    s->valid       = false;
    s->read_count  = 0;
    s->error_count = 0;
    s->retry_count = 0;
    s->state       = SENSOR_STATE_INIT;
    printf("[SENSOR] Initialized. State: %s\n",
           sensor_state_str(s->state));
    return SENSOR_OK;
}

sensor_err_t sensor_read(sensor_data_t *s) {
    if (!s) return SENSOR_ERR_I2C;
    sensor_err_t err = SENSOR_OK;
    float temp = 0, hum = 0;

    /* State machine */
    s->state = SENSOR_STATE_READ;
    printf("[SENSOR] State: %s | ", sensor_state_str(s->state));

    /* Try to read from I2C */
    err = i2c_read_sensor(&temp, &hum);

    if (err != SENSOR_OK) {
        s->state = SENSOR_STATE_ERROR;
        s->error_count++;
        s->valid = false;
        printf("ERR=%s | errors=%u\n",
               sensor_err_str(err), s->error_count);

        /* Retry logic */
        if (s->retry_count < SENSOR_MAX_RETRIES) {
            s->retry_count++;
            s->state = SENSOR_STATE_RETRY;
            printf("[SENSOR] State: %s | attempt %u/%u\n",
                   sensor_state_str(s->state),
                   s->retry_count, SENSOR_MAX_RETRIES);
            /* Retry once */
            err = i2c_read_sensor(&temp, &hum);
            if (err != SENSOR_OK) {
                printf("[SENSOR] Retry failed. ERR=%s\n",
                       sensor_err_str(err));
                return err;
            }
        } else {
            s->retry_count = 0;
            return SENSOR_ERR_MAX_RETRY;
        }
    }

    /* Validate range */
    s->state = SENSOR_STATE_VALIDATE;
    if (!validate_range(temp, hum)) {
        s->state = SENSOR_STATE_ERROR;
        s->error_count++;
        s->valid = false;
        printf("State: %s | ERR=OUT_OF_RANGE\n",
               sensor_state_str(s->state));
        return SENSOR_ERR_RANGE;
    }

    /* Success */
    s->temperature = temp;
    s->humidity    = hum;
    s->valid       = true;
    s->read_count++;
    s->retry_count = 0;
    printf("Temp=%.2fC Hum=%.2f%% reads=%u errors=%u\n",
           temp, hum, s->read_count, s->error_count);
    return SENSOR_OK;
}
