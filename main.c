#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "uart_hal.h"
#include "sensor_driver.h"
#include "watchdog.h"
#define TOTAL_READS 15
#define LOG_INTERVAL 3
int main(void) {
    printf("\n========================================\n");
    printf("  Embedded HAL Monitor - Linux Host\n");
    printf("  UART HAL + Sensor FSM + Watchdog\n");
    printf("========================================\n\n");
    uart_dev_t uart;
    if (uart_init(&uart, 115200) != UART_OK) { printf("UART init failed\n"); return -1; }
    sensor_data_t sensor;
    if (sensor_init(&sensor) != SENSOR_OK) { printf("Sensor init failed\n"); return -1; }
    watchdog_t wdg;
    wdg_init(&wdg);
    wdg_register_task(&wdg, "SensorTask", 3000);
    wdg_register_task(&wdg, "LogTask", 5000);
    printf("\n[MAIN] Starting %d reads...\n\n", TOTAL_READS);
    char buf[128]; int log_count=0;
    for (int i=0; i<TOTAL_READS; i++) {
        wdg_kick(&wdg, "SensorTask");
        sensor_err_t err = sensor_read(&sensor);
        if (err==SENSOR_OK && sensor.valid) {
            if (i % LOG_INTERVAL == 0) {
                wdg_kick(&wdg, "LogTask"); log_count++;
                snprintf(buf, sizeof(buf), "\n[UART_LOG #%d] Temp=%.2fC Hum=%.2f%% reads=%u errors=%u\n",
                    log_count, sensor.temperature, sensor.humidity, sensor.read_count, sensor.error_count);
                uart_puts(&uart, buf);
            }
        } else {
            snprintf(buf, sizeof(buf), "[UART_LOG] Sensor error: %s\n", sensor_err_str(err));
            uart_puts(&uart, buf);
        }
        wdg_check(&wdg);
        usleep(100000);
    }
    printf("\n========================================\n");
    printf("  FINAL REPORT\n");
    printf("  Total reads   : %u\n", sensor.read_count);
    printf("  Total errors  : %u\n", sensor.error_count);
    printf("  UART errors   : %u\n", uart_get_err_count(&uart));
    printf("  WDG checks    : %u\n", wdg.check_count);
    printf("  Success rate  : %.1f%%\n",
        (float)sensor.read_count/(sensor.read_count+sensor.error_count)*100.0f);
    printf("========================================\n");
    return 0;
}
