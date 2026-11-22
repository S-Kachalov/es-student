#include "api.h"
#include "pico/stdlib.h"
#include "stdio-text-protocol.h"
#include <string.h>

int main() {
  stdio_init_all();
  // led_init();
  // profiling_init();
  // stdio_text_protocol_init();

  while (1) {
    // принятая строка становится командой, команда — действием прибора
    const command_t *command = stdio_text_protocol_handle();
    if (command != NULL) {
      api_handle(command);
    }
  }
}
