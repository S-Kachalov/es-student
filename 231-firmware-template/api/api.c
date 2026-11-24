#include "api/api.h"
#include "button-task/button-task.h"
#include "device/device.h"
#include "hardware/timer.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "pico/types.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);

// команда прибора: имя, справка для help и обработчик
typedef struct {
  const char *name;
  const char *help;
  api_callback_t callback;
} api_command_t;

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
  printf("board: %s\n", info.board);
  printf("serial: %s\n", info.serial);
  printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
         (unsigned)info.manufacturer, (unsigned)info.part,
         (unsigned)info.revision);
  printf("pico-sdk: %s\n", info.sdk_version);
}

static void profiling(const command_t *command) {
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
  printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static void led_enable(const command_t *command) {
  led_task_set_state(LED_STATE_ON);
  print_response();
}

static void led_disable(const command_t *command) {
  led_task_set_state(LED_STATE_OFF);
  print_response();
}

static void led_blink(const command_t *command) {
  led_task_set_state(LED_STATE_BLINK);
  print_response();
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
  print_response();
}

static void button(const command_t *command) {
  if (button_task_is_pressed()) {
    printf("button: pressed, presses %u\n",
           (unsigned)button_task_get_press_count());
  } else {
    printf("button: released, presses %u\n",
           (unsigned)button_task_get_press_count());
  }
}

static void pi_start(const command_t *command) {
  uint32_t terms;

  if (command->argc != 2 || !parse_u32(command->argv[0], &terms)) {
    printf("error: usage pi_start [<terms>]\n");
    return;
  }
  if (terms == 0) {
    printf("error: terms must be greater than 0\n");
    return;
  }
  if (pi_task_start(terms)) {
    printf("pi: started, %u terms\n", (unsigned)terms);
  } else {
    printf("error: pi is busy\n");
  }
}

static void pi(const command_t *command) {
  double pi;
  uint64_t time;
  // Ответить значением или сообщить, что расчёт ещё идёт
  if (pi_task_get_result(&pi, &time)) {
    printf("pi: %.9f in %llu ms\n", pi, time / 1000);
    return;
  }
  if (pi_task_get_state() == NOT_STARTED) {
    printf("pi : not started\n");
    return;
  } else if (pi_task_get_state() == RUNNING) {
    printf("pi : running\n");
    return;
  }
}

static const api_command_t api_commands[] = {
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

static uint api_command_count =
    (sizeof(api_commands) / sizeof(api_commands[0]));

//****************************************************************************************************/

void api_handle(const command_t *command) {
  if (command->truncated) {
    printf("error: command longer than 63 characters or 4 arguments\n");
    return;
  }
  if (strcmp(command->name, "help") == 0) {
    printf("%-17s %s\n", "help", "list of commands");
    for (uint32_t i = 0; i < api_command_count; i++) {
      printf("%-17s %s\n", api_commands[i].name, api_commands[i].help);
    }
    return;
  }
  for (uint32_t i = 0; i < api_command_count; i++) {
    if (strcmp(command->name, api_commands[i].name) == 0) {
      api_commands[i].callback(command);
      return;
    }
  }
  printf("error: unknown command '%s', try help\n", command->name);
}
