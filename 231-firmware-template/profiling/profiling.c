#include "profiling.h"
#include "pico/stdlib.h"

// Внутренний массив секундомеров
static profiling_result_t stopwatches[STOPWATCH_COUNT];

// Массив для хранения последнего времени старта (в микросекундах)
static uint32_t start_time_us[STOPWATCH_COUNT];

// Флаг, запущен ли сейчас замер для конкретного ID
static bool is_running[STOPWATCH_COUNT];



void profiling_stopwatch_init(uint32_t id, const char *name) {
    stopwatches[id].name = name;
    stopwatches[id].count = 0;
    stopwatches[id].sum_us = 0;
    stopwatches[id].max_us = 0;

    is_running[id] = false;
    start_time_us[id] = 0;
}

void profiling_start(uint32_t id) {
    if (is_running[id]) return; // Защита от двойного старта

    start_time_us[id] = time_us_32();
    is_running[id] = true;
}

void profiling_stop(uint32_t id) {
    if (!is_running[id]) return;

    uint32_t now_us = time_us_32();
    uint32_t duration_us = now_us - start_time_us[id];

    // Обновляем статистику
    stopwatches[id].count++;
    stopwatches[id].sum_us += duration_us;

    if (duration_us > stopwatches[id].max_us) {
        stopwatches[id].max_us = duration_us;
    }

    is_running[id] = false;
}

bool profiling_get(uint32_t id, profiling_result_t *result) {
    if (stopwatches[id].name == NULL) {
        return false;
    }

    *result = stopwatches[id];
    return true;
}

void profiling_reset(void) {
    for (uint32_t i = 0; i < STOPWATCH_COUNT; i++) {
        // Сбрасываем только статистику, имена оставляем
        stopwatches[i].count = 0;
        stopwatches[i].sum_us = 0;
        stopwatches[i].max_us = 0;
        is_running[i] = false;
        start_time_us[i] = 0;
    }
}