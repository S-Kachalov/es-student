#include "logic/button-task/button-task.h"
#include "systime/systime.h"
#include <stdint.h>
#include <stdio.h>

typedef enum {
  BUTTON_STATE_RELEASED,
  BUTTON_STATE_PRESSING,
  BUTTON_STATE_PRESSED,
  BUTTON_STATE_RELEASING,
} button_state_t;

typedef struct {
  platform_button_t pin;
  button_state_t state;
  uint32_t press_count;
  uint64_t start_time_us;
  button_task_callback_t on_press;
} button_t;

static const uint32_t DEBOUNCE_US = 20000;

// Внутренний массив кнопок
static button_t pico_buttons[BUTTON_COUNT] = {
    {BUTTON_PIN_1, BUTTON_STATE_RELEASED, 0, 0, NULL},
    {BUTTON_PIN_2, BUTTON_STATE_RELEASED, 0, 0, NULL},
};

static button_t *button_find(platform_button_t pin) {
  for (uint32_t i = 0; i < BUTTON_COUNT; ++i) {
    if (pico_buttons[i].pin == pin) {
      return &pico_buttons[i];
    }
  }
  return NULL;
}

void button_task_handle(void) {
  uint64_t now_us = systime_us(); // запоминаем время при входе

  for (uint32_t i = 0; i < BUTTON_COUNT; ++i) {
    button_t *btn = &pico_buttons[i];

    // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль,
    // поэтому везде сравниваем с !level
    bool level = platform_button_read(btn->pin);

    switch (btn->state) {
    case BUTTON_STATE_RELEASED: {
      if (!level) {
        // «отпущена» -> «нажатие?»
        btn->state = BUTTON_STATE_PRESSING;
        btn->start_time_us = now_us;
      }
      break;
    }

    case BUTTON_STATE_PRESSING: {
      if (level) {
        // дребезг, возврат в «отпущена»
        btn->state = BUTTON_STATE_RELEASED;
      } else if (systime_period_elapsed(&btn->start_time_us, DEBOUNCE_US)) {
        // «нажатие?» -> «нажата», засчитываем нажатие
        btn->state = BUTTON_STATE_PRESSED;
        btn->press_count++;
        if (btn->on_press != NULL) {
          btn->on_press();
        }
      }
      break;
    }

    case BUTTON_STATE_PRESSED: {
      if (level) {
        // «нажата» -> «отпускание?»
        btn->state = BUTTON_STATE_RELEASING;
        btn->start_time_us = now_us;
      }
      break;
    }

    case BUTTON_STATE_RELEASING: {
      if (!level) {
        // дребезг при отпускании, возврат в «нажата»
        btn->state = BUTTON_STATE_PRESSED;
      } else if (systime_period_elapsed(&btn->start_time_us, DEBOUNCE_US)) {
        // «отпускание?» -> «отпущена»
        btn->state = BUTTON_STATE_RELEASED;
      }
      break;
    }
    }
  }
}

void button_task_init(platform_button_t button,
                      button_task_callback_t on_press) {
  button_t *btn = button_find(button);
  if (btn != NULL) {
    btn->on_press = on_press;
  }
}

bool button_task_is_pressed(platform_button_t button) {
  button_t *btn = button_find(button);
  if (btn == NULL) {
    return false;
  }
  return btn->state == BUTTON_STATE_PRESSED;
}

uint32_t button_task_get_press_count(platform_button_t button) {
  button_t *btn = button_find(button);
  if (btn == NULL) {
    return false;
  }
  return btn->press_count;
}
