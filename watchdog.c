#include <stdio.h>
#include <string.h>
#include <time.h>
#include "watchdog.h"
static uint32_t get_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
wdg_err_t wdg_init(watchdog_t *w) {
    if (!w) return WDG_ERR_TIMEOUT;
    memset(w, 0, sizeof(watchdog_t));
    printf("[WDG] Watchdog initialized\n");
    return WDG_OK;
}
wdg_err_t wdg_register_task(watchdog_t *w, const char *name, uint32_t timeout_ms) {
    if (w->task_count >= MAX_TASKS) return WDG_ERR_TIMEOUT;
    wdg_task_t *t = &w->tasks[w->task_count++];
    t->name=name; t->timeout_ms=timeout_ms; t->last_kick=get_ms(); t->active=true;
    printf("[WDG] Registered task: %s timeout=%ums\n", name, timeout_ms);
    return WDG_OK;
}
wdg_err_t wdg_kick(watchdog_t *w, const char *name) {
    for (int i=0; i<w->task_count; i++)
        if (strcmp(w->tasks[i].name, name)==0) { w->tasks[i].last_kick=get_ms(); return WDG_OK; }
    return WDG_ERR_TIMEOUT;
}
wdg_err_t wdg_check(watchdog_t *w) {
    uint32_t now=get_ms(); w->check_count++;
    for (int i=0; i<w->task_count; i++) {
        wdg_task_t *t=&w->tasks[i];
        if (!t->active) continue;
        uint32_t elapsed=now-t->last_kick;
        if (elapsed>t->timeout_ms) {
            printf("[WDG] FAULT: Task '%s' hung! elapsed=%ums\n", t->name, elapsed);
            return WDG_ERR_TASK_HANG;
        }
    }
    return WDG_OK;
}
