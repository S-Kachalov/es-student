#include "button-task/button-task.h"
#include "hardware/gpio.h"
#include "led-task/led-task.h"
#include "systime/systime.h"

typedef enum {
  BUTTON_STATE_RELEASED,
  BUTTON_STATE_PRESSING,
  BUTTON_STATE_PRESSED,
  BUTTON_STATE_RELEASING,
} button_state_t;

static uint64_t start_time_us = 0;
static const uint32_t DEBOUNCE_US = 20000;
static const uint32_t BUTTON_PIN = 15;
static uint32_t press_count = 0;
static button_state_t button_state;

void button_task_handle(void) {
  // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
  // т.е. везде где проверяем на нажатие на !level в if condition
  bool level = gpio_get(BUTTON_PIN);

  // читаем часы
  uint64_t now_us = systime_us();

  // переходы по таблице; при переходе в «нажата»
  // увеличиваем счётчик и вызываем led_task_next_state()
  switch (button_state) {
  case BUTTON_STATE_RELEASED: {
    if (!level) {
      // Переход: «отпущена» -> «нажатие?»
      button_state = BUTTON_STATE_PRESSING;
      start_time_us = now_us;
    }
    break;
  }

  case BUTTON_STATE_PRESSING: {
    if (level) {
      // Возврат в «отпущена»: был дребезг
      button_state = BUTTON_STATE_RELEASED;
    } else if (systime_period_elapsed(&start_time_us, DEBOUNCE_US)) {
      // Переход: «нажатие?» -> «нажата», засчитываем нажатие
      button_state = BUTTON_STATE_PRESSED;
      press_count++;
      led_task_next_state();
    }
    break;
  }

  case BUTTON_STATE_PRESSED: {
    if (level) {
      // Переход: «нажата» -> «отпускание?»
      button_state = BUTTON_STATE_RELEASING;
      start_time_us = now_us;
    }
    // Если кнопка снова нажата - остаёмся в «нажата»
    break;
  }

  case BUTTON_STATE_RELEASING: {
    if (!level) {
      // Возврат в «нажата»: был дребезг при отпускании
      button_state = BUTTON_STATE_PRESSED;
    } else if (systime_period_elapsed(&start_time_us, DEBOUNCE_US)) {
      // Переход: «отпускание?» -> «отпущена»
      button_state = BUTTON_STATE_RELEASED;
    }
    break;
  }
  }
}

void button_task_init(void) {
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);
}

bool button_task_is_pressed(void) {
  return (button_state == BUTTON_STATE_PRESSED);
}

uint32_t button_task_get_press_count(void) { return press_count; }
