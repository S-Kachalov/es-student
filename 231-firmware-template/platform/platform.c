#include "platform/platform.h"
#include "pico/stdlib.h"
static const uint32_t BUTTON_PIN = 15;

// запускает стандартный ввод-вывод и настраивает выводы
void platform_init(void) {
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  stdio_init_all();
}

void platform_led_set(bool on) { gpio_put(PICO_DEFAULT_LED_PIN, on); }

bool platform_led_get(void) { return gpio_get(PICO_DEFAULT_LED_PIN); }

bool platform_button_read(void) { return gpio_get(BUTTON_PIN); }
