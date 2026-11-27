#include "logic/telemetry-task/telemetry-task.h"
#include "platform/platform.h"
#include "systime/systime.h"
#include <stddef.h>

static telemetry_record_t record;
static uint32_t period;
static telemetry_state_t state;
static uint64_t start_time_us = 0;

static telemetry_task_callback_t send_record;

void telemetry_task_init(telemetry_task_callback_t on_record) {
  //: запомнить функцию для записей, состояние IDLE
  if (on_record != NULL) {
    send_record = on_record;
  }
  state = TELEMETRY_STATE_IDLE;
}

/*
  Первая запись сработает на первой же итерации суперцикла:
  разность now - (now - period) равна периоду ровно,
  условие now_us - *last_us < period_us даёт false,
  функция вернёт true и корректно выставит *last_us = now
*/
// telemetry_task_start(): период меньше 10 мс не принять,
// запомнить период, первую запись — на ближайшей итерации, состояние RUN
bool telemetry_task_start(uint32_t period_ms) {
  if (period_ms < 10) {
    return false;
  }
  period = period_ms;
  // сдвигаем точку отсчёта в прошлое, чтобы первая запись ушла сразу
  start_time_us = systime_us() - (uint64_t)period * 1000;
  // record.time_us = 0;
  record.voltage_v = 0;
  record.temp_c = 0;
  state = TELEMETRY_STATE_RUN;
  return true;
}

// telemetry_task_stop(): состояние IDLE
void telemetry_task_stop(void) { state = TELEMETRY_STATE_IDLE; }

void telemetry_task_handle(void) {
  // в состоянии RUN, если прошёл период:
  // записать время systime_us(), напряжение и температуру у platform
  // отдать запись функции, полученной при запуске
  switch (state) {
  case TELEMETRY_STATE_IDLE:
    break;
  case TELEMETRY_STATE_RUN:
    if (systime_period_elapsed(&start_time_us, (uint64_t)period * 1000)) {
      // record.time_us = start_time_us;
      record.time_us = systime_us();
      record.voltage_v = platform_voltage_read();
      record.temp_c = platform_temp_read();
      if (send_record != NULL) {
        send_record(&record);
      }
    }
    break;
  }
}

telemetry_state_t telemetry_task_get_state(void) { return state; }

uint32_t telemetry_task_get_period_ms(void) { return period; }
