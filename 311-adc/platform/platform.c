#include "platform/platform.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

static const uint32_t BUTTON_PIN = 15;

// вход АЦП: вывод GP26, канал 0
#define ADC_PIN 26
#define ADC_CHANNEL 0

// опорное напряжение АЦП на плате Pico и число уровней 12-разрядного кода
#define ADC_VREF_V 3.3f
#define ADC_LEVELS 4096

// запускает стандартный ввод-вывод и настраивает выводы
void platform_init(void) {
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  stdio_init_all();

  adc_init();
  adc_gpio_init(ADC_PIN);
}

void platform_led_set(bool on) { gpio_put(PICO_DEFAULT_LED_PIN, on); }

bool platform_led_get(void) { return gpio_get(PICO_DEFAULT_LED_PIN); }

bool platform_button_read(void) { return gpio_get(BUTTON_PIN); }

float platform_voltage_read(void) {
  adc_select_input(ADC_CHANNEL);
  return adc_read() * ADC_VREF_V / ADC_LEVELS;
}
