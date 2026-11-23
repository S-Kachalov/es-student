#include "api/api.h"
#include "device.h"
#include "hardware/timer.h"
#include "led-task.h"
#include "profiling.h"
#include "stdio-text-protocol.h"
#include <ctype.h>
#include <errno.h>
#include <pico/types.h>
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

static double calc_pi(uint terms) {
  double sum = 0.0;
  double sign = 1.0;

  for (int k = 0; k < terms; k++) {
    // Прибавить очередной член ряда: ±1 / (2k + 1)
    sum += sign / (2.0 * k + 1.0);
    sign = -sign;
  }

  // Сумма ряда равна π/4
  return 4.0 * sum;
}

static void cmd_info(const command_t *command) {
  // печатаем паспорт устройства
  device_info();
}

static void main_time_exec(const command_t *command) {
  printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(),
         (unsigned)profiling_max_us());
}

static void main_time_reset(const command_t *command) {
  profiling_reset_max();
  printf("max reset\n");
}

static void uptime(const command_t *command) {
  printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static void cmd_calc_pi(const command_t *command) {
  uint32_t terms;
  if (command->argc != 2 || !parse_u32(command->argv[0], &terms)) {
    printf("error: usage calc_pi [terms]\n");
    return;
  }
  uint64_t start_us = time_us_64();
  volatile double pi_result = calc_pi(terms);
  uint64_t spent_us = time_us_64() - start_us;

  printf("pi: %.8f\n", pi_result);
  printf("time: %llu ms\n", spent_us / 1000);
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

static const api_command_t api_commands[] = {
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", uptime},
    {"calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi},
    {"main_time_exec", "superloop iteration: average and maximum",
     main_time_exec},
    {"main_time_reset", "reset maximum iteration", main_time_reset},
    {"led_enable", "switch on led", led_enable},
    {"led_disable", "switch off led", led_disable},
    {"led_blink", "start led blinking", led_blink},
    {"led_period", "sets period of led blinkiking", led_period},
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
