#ifndef WATCHDOG_H
#define WATCHDOG_H
#include <stdint.h>
#include <stdbool.h>
#define MAX_TASKS 4
#define WDG_TIMEOUT_MS 2000
typedef enum { WDG_OK=0, WDG_ERR_TIMEOUT, WDG_ERR_TASK_HANG } wdg_err_t;
typedef struct { const char *name; uint32_t last_kick; uint32_t timeout_ms; bool active; } wdg_task_t;
typedef struct { wdg_task_t tasks[MAX_TASKS]; uint8_t task_count; uint32_t check_count; } watchdog_t;
wdg_err_t wdg_init(watchdog_t *w);
wdg_err_t wdg_register_task(watchdog_t *w, const char *name, uint32_t timeout_ms);
wdg_err_t wdg_kick(watchdog_t *w, const char *name);
wdg_err_t wdg_check(watchdog_t *w);
#endif
