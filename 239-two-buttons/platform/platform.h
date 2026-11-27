#pragma once

#include <stdbool.h>

#define BUTTON_COUNT 2

typedef enum {
  BUTTON_PIN_1 = 14, // номер пина кнопки 1
  BUTTON_PIN_2 = 15, // номер пина кнопки 2
} platform_button_t;

// запускает стандартный ввод-вывод и настраивает выводы
void platform_init(void);

void platform_led_set(bool on);
bool platform_led_get(void);

// нажата ли кнопка сейчас, без подавления дребезга
bool platform_button_read(platform_button_t button_pin);
