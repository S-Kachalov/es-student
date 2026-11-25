#include "led-task/led-task.h"
#include "hardware/gpio.h"
#include "systime/systime.h"

// Состояние задачи: внутреннее связывание, только для этого файла
static led_state_t led_state;
static uint64_t half_period_us;
static uint64_t last_toggle_us = 0;
bool led_on = false;
bool led_blinking = false;

// Вспомогательная функция: из других файлов её не вызвать
static void led_set(bool on) { gpio_put(PICO_DEFAULT_LED_PIN, on); }

bool led_task_set_period_ms(uint32_t period_ms) {
  // С нулевым полупериодом светодиод переключался бы
  // на каждой итерации: такой период не принимается
  if (period_ms == 0) {
    return false;
  }

  half_period_us = (uint64_t)period_ms * 1000 / 2;
  return true;
}

uint32_t led_task_get_period_ms(void) { return half_period_us * 2; }

void led_task_init(void) {
  // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  led_task_set_period_ms(1000);
  led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state) {
  // запоминаем состояние и выполняем действие при входе:
  led_state = state;
  switch (led_state) {
  case LED_STATE_OFF:
    led_set(false);
    break;
  case LED_STATE_ON:
    led_set(true);
    break;
  case LED_STATE_BLINK:
    // Первое переключение — на ближайшей итерации
    last_toggle_us = 0;
    break;
  }
}

led_state_t led_task_get_state(void) { return led_state; }

void led_task_handle(void) {
  switch (led_state) {
    // «выключен» и «горит» — ничего не делаем, светодиод установлен при входе
    // «мигает» — каждые полпериода переключаем светодиод
  case LED_STATE_OFF:
  case LED_STATE_ON:
    // Светодиод установлен при входе в состояние, делать нечего
    break;
  case LED_STATE_BLINK:
    if (systime_period_elapsed(&last_toggle_us, (half_period_us * 2))) {
      gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
    }
    break;
  }
}

void led_task_next_state(void) {
  led_state_t led_state = led_task_get_state();
  led_state++;
  if (led_state > LED_STATE_BLINK) {
    led_state = 0;
  }
  led_task_set_state(led_state);
}
