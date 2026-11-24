#pragma once

#include <stdbool.h>
#include <stdint.h>

// Ограничение: sum_us — 32-битная сумма замеров.
// При типичной длительности замера ~10 мкс,
// переполнение наступит примерно через 71 минуту непрерывной работы.
// Если нужна большая длительность — увеличьте тип sum_us до uint64_t.
#define STOPWATCH_COUNT 5

// Результат замера для одного секундомера.
typedef struct {
  const char *name; // Имя секундомера
  uint32_t count;   // Число замеров
  uint32_t sum_us;  // Сумма всех замеров (в мкс) (32 бита)
  uint32_t max_us;  // Максимум среди замеров (в мкс)
} profiling_result_t;

// Инициализация конкретного секундомера по ID.
void profiling_stopwatch_init(uint32_t id, const char *name);

// Запуск замера по ID секундомера. Запоминает текущее время.
void profiling_start(uint32_t id);

// Остановка замера по ID секундомера. Вычисляет длительность,
// обновляет статистику (count, sum_us, max_us).
void profiling_stop(uint32_t id);

// Получение текущих результатов по ID.
bool profiling_get(uint32_t id, profiling_result_t *result);

// Сброс всех секундомеров: обнуляет счётчики, суммы и максимумы.
// Имена остаются.
void profiling_reset(void);
