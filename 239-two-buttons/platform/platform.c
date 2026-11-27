#include "platform/platform.h"
#include "pico/stdlib.h"

static const platform_button_t button_gpios[BUTTON_COUNT] = {
    14, // кнопка 1
    15, // кнопка 2
};

// запускает стандартный ввод-вывод и настраивает выводы
void platform_init(void) {
  for (uint i = 0; i < BUTTON_COUNT; i++) {
    gpio_init(button_gpios[i]);
    gpio_set_dir(button_gpios[i], GPIO_IN);
    gpio_pull_up(button_gpios[i]);
  }

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  stdio_init_all();
}

void platform_led_set(bool on) { gpio_put(PICO_DEFAULT_LED_PIN, on); }

bool platform_led_get(void) { return gpio_get(PICO_DEFAULT_LED_PIN); }

bool platform_button_read(platform_button_t button) {
  return gpio_get(button_gpios[button]);
}
