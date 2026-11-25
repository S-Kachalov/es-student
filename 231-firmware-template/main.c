#include "api/api.h"
#include "button-task/button-task.h"
#include "firmware.h"
#include "led-task/led-task.h"
#include "logging/log.h"
#include "pi-task/pi-task.h"
#include "profiling/profiling.h"
#include "stdio_text_protocol/stdio-text-protocol.h"
#include <stddef.h>

enum {
  STOPWATCH_LOOP,
  STOPWATCH_COMMAND,
  STOPWATCH_LED,
  STOPWATCH_BUTTON,
  STOPWATCH_PI,
};

int main() {
  stdio_text_protocol_init();
  led_task_init();
  button_task_init();

  profiling_stopwatch_init(STOPWATCH_LOOP, "loop");
  profiling_stopwatch_init(STOPWATCH_COMMAND, "command");
  profiling_stopwatch_init(STOPWATCH_LED, "led");
  profiling_stopwatch_init(STOPWATCH_BUTTON, "button");
  profiling_stopwatch_init(STOPWATCH_PI, "pi");

  LOG_INF("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);

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

    profiling_stop(STOPWATCH_LOOP);
  }
}
