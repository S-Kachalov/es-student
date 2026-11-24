#include "api/api.h"
#include "button-task/button-task.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include <string.h>

int main() {
  stdio_text_protocol_init();
  led_task_init();
  button_task_init();

  while (1) {
    // принятая строка становится командой, команда — действием прибора
    const command_t *command = stdio_text_protocol_handle();
    if (command != NULL) {
      api_handle(command);
    }
    led_task_handle();
    button_task_handle();
    pi_task_handle();
  }
}
