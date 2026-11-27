#include "firmware.h"
#include "logging/log.h"
#include "logic/api/api.h"
#include "logic/button-task/button-task.h"
#include "logic/led-task/led-task.h"
#include "logic/pi-task/pi-task.h"
#include "logic/telemetry-task/telemetry-task.h"
#include "platform/platform.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include <stddef.h>

enum {
  STOPWATCH_LOOP,
  STOPWATCH_COMMAND,
  STOPWATCH_LED,
  STOPWATCH_BUTTON,
  STOPWATCH_PI,
  STOPWATCH_TELEMETRY,
};

int main() {
  platform_init();

  LOG_INF("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);

  led_task_init();
  button_task_init(led_task_next_state);
  telemetry_task_init(api_print_telemetry);

  profiling_stopwatch_init(STOPWATCH_LOOP, "loop");
  profiling_stopwatch_init(STOPWATCH_COMMAND, "command");
  profiling_stopwatch_init(STOPWATCH_LED, "led");
  profiling_stopwatch_init(STOPWATCH_BUTTON, "button");
  profiling_stopwatch_init(STOPWATCH_PI, "pi");
  profiling_stopwatch_init(STOPWATCH_TELEMETRY, "telemetry");

  while (1) {
    profiling_start(STOPWATCH_LOOP);

    // --- Приём и обработка команды ---
    profiling_start(STOPWATCH_COMMAND);
    const command_t *command = stdio_text_protocol_handle();
    if (command != NULL) {
      api_handle(command);
    }
    profiling_stop(STOPWATCH_COMMAND);

    // --- Задача светодиода ---
    profiling_start(STOPWATCH_LED);
    led_task_handle();
    profiling_stop(STOPWATCH_LED);

    // --- Задача кнопки ---
    profiling_start(STOPWATCH_BUTTON);
    button_task_handle();
    profiling_stop(STOPWATCH_BUTTON);

    // --- Расчёт числа Пи ---
    profiling_start(STOPWATCH_PI);
    pi_task_handle();
    profiling_stop(STOPWATCH_PI);

    // --- Отправка телеметрии ---
    profiling_start(STOPWATCH_TELEMETRY);
    telemetry_task_handle();
    profiling_stop(STOPWATCH_TELEMETRY);

    profiling_stop(STOPWATCH_LOOP);
  }
}
