#include "device/device.h"
#include "logic/api/api-commands.h"
#include "logic/button-task/button-task.h"
#include "logic/led-task/led-task.h"
#include "logic/pi-task/pi-task.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "systime/systime.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Число без знака из десятичной записи. false — в записи не только цифры
// или число не помещается в uint32_t.
static bool parse_u32(const char *text, uint32_t *value) {
  // strtoul приняла бы и знак: из "-1" получилось бы 4294967295
  if (!isdigit((unsigned char)text[0])) {
    return false;
  }

  char *end;
  errno = 0;
  unsigned long number = strtoul(text, &end, 10);

  // после числа остались символы или число не поместилось
  if (*end != '\0' || errno == ERANGE || number > UINT32_MAX) {
    return false;
  }

  *value = (uint32_t)number;
  return true;
}

static void cmd_info(const command_t *command) {
  // печатаем паспорт устройства
  device_info_t info;
  device_get_info(&info);
  printf("name: %s\n", info.name);
  printf("version: %s\n", info.version);
  printf("project: %s\n", info.project);
  printf("repo: %s\n", info.repo);
  printf("board: %s\n", info.board);
  printf("serial: %s\n", info.serial);
  printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
         (unsigned)info.manufacturer, (unsigned)info.part,
         (unsigned)info.revision);
  printf("pico-sdk: %s\n", info.sdk_version);
}

static void profiling(const command_t *command) {
  if (!PROFILING_ENABLED) {
    printf("profiling is disabled in this build\n");
    return;
  }
  for (int i = 0; i < STOPWATCH_COUNT; i++) {
    profiling_result_t res;
    if (profiling_get(i, &res)) {
      if (res.count == 0) {
        // Если замеров не было, среднее не считаем, чтобы не делить на 0
        printf("%-10s count %-10u mean 0.00 us  max 0 us\n", res.name,
               (unsigned)res.count);
      } else {
        float mean_us = (float)res.sum_us / (float)res.count;
        printf("%-10s count %-10u mean %.2f us  max %u us\n", res.name,
               (unsigned)res.count, mean_us, (unsigned)res.max_us);
      }
    }
  }
}

static void profiling_reset_cmd(const command_t *command) {
  profiling_reset();
  printf("profiling reset\n");
}

static void uptime(const command_t *command) {
  printf("uptime: %u ms\n", (unsigned)systime_ms());
}

static void led_enable(const command_t *command) {
  led_task_set_state(LED_STATE_ON);
  printf("led: %s, period %u ms\n", "on",
         ((unsigned)led_task_get_period_ms() / 1000));
}

static void led_disable(const command_t *command) {
  led_task_set_state(LED_STATE_OFF);
  printf("led: %s, period %u ms\n", "off",
         ((unsigned)led_task_get_period_ms() / 1000));
}

static void led_blink(const command_t *command) {
  led_task_set_state(LED_STATE_BLINK);
  printf("led: %s, period %u ms\n", "blink",
         ((unsigned)led_task_get_period_ms() / 1000));
}

static void led_period(const command_t *command) {
  uint32_t period;
  if (command->argc != 2 || !parse_u32(command->argv[0], &period)) {
    printf("error: usage led_period <period_ms>\n");
    return;
  }
  if (period == 0) {
    printf("error: period_ms must be greater than 0\n");
    return;
  }
  led_task_set_period_ms(period);
  switch (led_task_get_state()) {
  case LED_STATE_OFF:
    printf("led: %s, period %u ms\n", "off",
           ((unsigned)led_task_get_period_ms() / 1000));
    break;
  case LED_STATE_ON:
    printf("led: %s, period %u ms\n", "on",
           ((unsigned)led_task_get_period_ms() / 1000));
    break;
  case LED_STATE_BLINK:
    printf("led: %s, period %u ms\n", "blink",
           ((unsigned)led_task_get_period_ms() / 1000));
    break;
  }
}

static void button(const command_t *command) {
  printf("button 1: %s, presses %u\n",
         button_task_is_pressed(BUTTON_PIN_1) ? "pressed" : "released",
         (unsigned)button_task_get_press_count(BUTTON_PIN_1));
  printf("button 2: %s, presses %u\n",
         button_task_is_pressed(BUTTON_PIN_2) ? "pressed" : "released",
         (unsigned)button_task_get_press_count(BUTTON_PIN_2));
}

static void pi_start(const command_t *command) {
  uint32_t terms = PI_TERMS_DEFAULT;

  if (command->argc > 2 ||
      (command->argc == 2 && !parse_u32(command->argv[0], &terms))) {
    printf("error: usage pi_start [<terms>]\n");
    return;
  }
  if (terms == 0) {
    printf("error: terms must be greater than 0\n");
    return;
  }
  if (!pi_task_start(terms)) {
    printf("error: pi is busy\n");
    return;
  }
  printf("pi: started, %u terms\n", (unsigned)terms);
}

static void pi(const command_t *command) {
  double value;
  uint64_t ms;

  if (pi_task_get_result(&value, &ms)) {
    printf("pi: %.9f in %llu ms\n", value, (unsigned long long)ms);
    return;
  }
  switch (pi_task_get_state()) {
  case RUNNING:
    printf("pi: running\n");
    break;
  default: // NOT_STARTED и любое неизвестное состояние
    printf("pi: not started\n");
    break;
  }
}

const api_command_t api_commands[] = {
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", uptime},
    {"profiling", "show all running stopwatches", profiling},
    {"profiling_reset", "reset all running stopwatches", profiling_reset_cmd},
    {"led_enable", "switch on led", led_enable},
    {"led_disable", "switch off led", led_disable},
    {"led_blink", "start led blinking", led_blink},
    {"led_period", "sets period of led blinkiking", led_period},
    {"button", "show button state", button},
    {"pi_start", "starts calculate pi", pi_start},
    {"pi", "show result of pi calculation or state", pi},
};

const uint32_t api_command_count =
    (sizeof(api_commands) / sizeof(api_commands[0]));
