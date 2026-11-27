#include "logic/pi-task/pi-task.h"
#include "systime/systime.h"
#include <stdint.h>

//  ценa члена ряда около 585 тактов. При 125 МГц это 4,7 мкс.
//  Обработчик задачи должен уложиться в 0,5 мс, а это около 100 членов
#define PI_TERMS_PER_CALL 100

// Состояние расчёта: должно пережить возврат из обработчика
static pi_state_t state = NOT_STARTED;
static bool pi_ready = false;
static uint32_t pi_k;
static uint32_t pi_n;
static double pi_sum;
static double pi_sign;
static double pi_result;
static uint64_t time_us;

pi_state_t pi_task_get_state(void) { return state; }

bool pi_task_start(int n) {
  if (state == RUNNING) {
    return false;
  }
  pi_n = n;
  pi_k = 0;
  pi_sum = 0.0;
  pi_sign = 1.0;
  pi_ready = false;
  time_us = 0; // сброс накопленного времени
  state = RUNNING;
  return true;
}

// Готов ли результат: если да, записать его по указателю
bool pi_task_get_result(double *result, uint64_t *time_spent) {
  if (!pi_ready) {
    return false;
  }
  *result = pi_result;
  *time_spent = time_us / 1000; // сразу в мс
  return true;
}

void pi_task_handle(void) {
  if (!(state == RUNNING)) {
    return;
  }

  // Очередная порция членов ряда, но не дальше последнего
  int end = pi_k + PI_TERMS_PER_CALL;
  if (end > pi_n) {
    end = pi_n;
  }
  uint64_t start_us = systime_us();
  for (; pi_k < end; pi_k++) {
    pi_sum += pi_sign / (2.0 * pi_k + 1.0);
    pi_sign = -pi_sign;
  }
  uint64_t spent_us = systime_us() - start_us;
  time_us += spent_us;
  if (pi_k == pi_n) {
    pi_result = 4.0 * pi_sum;
    state = DONE;
    pi_ready = true;
  }
};
