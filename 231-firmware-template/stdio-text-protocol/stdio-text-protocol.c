#include "stdio-text-protocol.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static uint line_length = 0;
static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];

const command_t *stdio_text_protocol_handle(void) {
  static command_t command; // переживёт возврат из функции

  // читаем символ без ожидания; нет символа - возвращаем NULL
  int symbol = getchar_timeout_us(0);
  if (symbol == PICO_ERROR_TIMEOUT) {
    return NULL;
  }

  // конец строки: пустую строку пропускаем, иначе переводим строку
  // в терминале, делим строку на слова и возвращаем команду
  if (symbol == '\r' || symbol == '\n') {
    putchar('\n');
    line[line_length] = '\0';

    if (line_length == 0) {
      return NULL; // пустая строка - пропускаем
    }

    command.name = NULL;
    command.argc = 0;
    command.truncated = false;

    // делим строку на слова
    char *p = line;
    while (*p != '\0') {
      // пробелы подряд превращаются в нули подряд
      while (*p == ' ') {
        *p = '\0';
        p++;
      }
      if (*p == '\0') {
        break;
      }
      // символ не пробел и перед ним ноль - начало слова
      if (command.argc == 0) { // argc == 0 значит это первое слово
        command.name = p;      // -> имя команды
      } else if (command.argc - 1 < STDIO_TEXT_PROTOCOL_MAX_ARGS) {
        command.argv[command.argc - 1] = p; // -> аргумент
      } else {
        // слова сверх четвёртого аргумента не сохраняются
        command.truncated = true;
      }
      command.argc++; // увеличиваем счётчик слов

      // доходим до конца слова
      while (*p != '\0' && *p != ' ') {
        p++;
      }
    }

    // очистка буфера
    line_length = 0;

    // строка из одних пробелов - слов нет, команды нет
    if (command.name == NULL) {
      return NULL;
    }

    return &command; // и возвращаем команду для api
  }

  // Backspace (0x08 или 0x7f): убираем последний символ из буфера
  // и стираем его на экране "\b \b"
  if ((symbol == 0x08) || (symbol == 0x7F)) {
    if (line_length > 0) {
      line_length--;
      putchar('\b');
      putchar(' ');
      putchar('\b');
    }
    return NULL;
  }

  // печатный символ: кладём в буфер и повторяем в терминал
  if ((symbol >= 0x20) && (symbol <= 0x7E)) {
    if (line_length < STDIO_TEXT_PROTOCOL_LINE_SIZE - 1) {
      line[line_length] = (char)symbol;
      line_length++;
      putchar(symbol);
    } else {
      // буфер полон — строка не поместится целиком
      command.truncated = true;
    }
  }
  // остальные символы игнорируем
  return NULL;
}
