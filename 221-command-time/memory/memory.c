#include "command.h"
#include "device.h"
#include "hardware/regs/addressmap.h"
#include "led.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

#define VECTOR_TABLE 0x10000100

uint32_t data_variable = 100;
uint32_t bss_variable;

static void row(const char *name, uintptr_t start, uintptr_t end) {
  printf("%-10s 0x%08x 0x%08x %8u\n", name, (unsigned)start, (unsigned)end,
         (unsigned)(end - start));
}

static unsigned size(uintptr_t start, uintptr_t end) {
  return (unsigned)(end - start);
}

void mem_info(void) {
  // шапка таблицы: область, начало, конец, размер
  printf("%-10s %-10s %-10s %-13s\n", "area", "start", "end", "size");

  // flash — XIP_BASE и PICO_FLASH_SIZE_BYTES
  row("flash", (uintptr_t)XIP_BASE,
      (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));

  // sram — базовый адрес из SDK - 0x20000000 , размер из datasheet - 256kB
  row("sram", (uintptr_t)SRAM_BASE, (uintptr_t)SRAM_END);

  // rom — базовый адрес из SDK - 0x00000000, размер из datasheet - 16kB
  row("rom", (uintptr_t)ROM_BASE, (uintptr_t)0x00004000);

  // image — от __flash_binary_start до __flash_binary_end
  row("image", (uintptr_t)&__flash_binary_start,
      (uintptr_t)&__flash_binary_end);

  // free  — от __flash_binary_end до конца флеш-памяти
  row("free", (uintptr_t)&__flash_binary_end,
      (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));

  // boot2 — от __boot2_start__ до __boot2_end__
  row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);

  // text  — от __boot2_end__ до __etext: код и константы
  row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

  // data flash — хранение .data, от __etext, длиной с .data
  row("data flash", (uintptr_t)&__etext, (uintptr_t)&__flash_binary_end);

  // data ram   — работа .data, от __data_start__ до __data_end__
  row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);

  // bss        — от __bss_start__ до __bss_end__
  row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);

  // heap       — от __bss_end__ до __HeapLimit
  row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);

  // stack      — от __StackBottom до __StackTop
  row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

  printf("total\n");
  // итог: образ во флеш и из чего он сложился
  printf("flash image %3u = boot2 %u + text %u + data %u\n",
         size((uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end),
         size((uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__),
         size((uintptr_t)&__boot2_end__, (uintptr_t)&__etext),
         size((uintptr_t)&__etext, (uintptr_t)&__flash_binary_end));

  // итог: свободно во флеш-памяти из всего её объёма
  printf("flash free %3u of %u\n",
         size((uintptr_t)&__flash_binary_end,
              (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES)),
         0x00200000);

  // итог: занято в ОЗУ — .data и .bss
  printf("ram used %3u = data %u + bss %u\n",
         (size((uintptr_t)&__data_start__, (uintptr_t)&__data_end__) +
          size((uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__)),
         size((uintptr_t)&__data_start__, (uintptr_t)&__data_end__),
         size((uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__));

  // итог: свободно в ОЗУ — под кучу и под стек
  printf("ram free %3u for heap and %u for stack\n",
         size((uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit),
         size((uintptr_t)&__StackBottom, (uintptr_t)&__StackTop));
}

int main(void);

void fw_info(void) {
  // считаем вызов: data_variable и bss_variable на единицу больше
  data_variable++;
  bss_variable++;

  // адреса функций со сброшенным признаком Thumb
  uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
  uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

  // локальная переменная и блок из кучи
  uint32_t stack_variable = 1946;
  uint32_t *heap_variable = malloc(sizeof(uint32_t));
  if (heap_variable != NULL) {
    *heap_variable = 1951;
  }

  // шапка: объект, адрес, значение
  printf("%-14s %-12s %-12s\n", "object", "address", "value");

  // main, fw_info  — адрес с признаком Thumb и два байта по сброшенному адресу
  printf("%-14s 0x%08x  0x%04x\n", "main", (unsigned)main, *main_code);
  printf("%-14s 0x%08x  0x%04x\n", "fw_info", (unsigned)fw_info, *fw_info_code);

  // commands       — адрес массива
  printf("%-14s 0x%08x\n", "commands", (unsigned)commands);

  // обработчики    — имя команды и адрес обработчика, строкой на команду
  for (uint i = 0; i < command_count; i++) {
    if (commands[i].handler != NULL) {
      printf("- %-12s 0x%08x\n", commands[i].name,
             (unsigned)commands[i].handler);
    }
  }

  // константы      — адрес и значение строк паспорта из device.h
  printf("%-14s 0x%08x  %s\n", "DEVICE_PROJECT", (unsigned)DEVICE_PROJECT,
         DEVICE_PROJECT);
  printf("%-14s 0x%08x  %s\n", "DEVICE_BOARD", (unsigned)DEVICE_BOARD,
         DEVICE_BOARD);

  // data_variable  — адрес и значение, секция .data
  printf("%-14s 0x%08x  %u\n", "data_variable", (unsigned)&data_variable,
         data_variable);

  // bss_variable   — адрес и значение, секция .bss
  printf("%-14s 0x%08x  %u\n", "bss_variable", (unsigned)&bss_variable,
         bss_variable);

  // stack_variable — адрес и значение
  printf("%-14s 0x%08x  %u\n", "stack_variable", (unsigned)&stack_variable,
         stack_variable);

  // heap_variable  — адрес и значение
  printf("%-14s 0x%08x  %u\n", "heap_variable", (unsigned)heap_variable,
         *heap_variable);

  // возвращаем блок кучи
  free(heap_variable);
}

void boot_info(void) {
  // указатель на таблицу векторов и два первых слова из неё
  const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;
  uint32_t stack_top = vectors[0];
  uint32_t reset_handler = vectors[1];

  // указатель на регистр GPIO_IN и разряд вывода светодиода
  volatile uint32_t *gpio_in = (uint32_t *)(0xd0000004);

  // vector table   — адрес таблицы
  printf("%-15s 0x%08x\n", "vector table", (unsigned)vectors);

  //   stack top    — первое слово
  printf("  %-13s 0x%08x\n", "stack top", (unsigned)vectors[0]);

  //   reset        — второе слово
  printf("  %-13s 0x%08x\n", "reset", (unsigned)vectors[1]);

  //   reset (even) — оно же со сброшенным признаком Thumb
  printf("  %-13s 0x%08x\n", "reset (even)",
         (unsigned)((uintptr_t)vectors[1] & ~1u));

  // gpio in        — адрес регистра
  printf("%-15s 0x%08x\n", "gpio in", (unsigned)gpio_in);

  //   led bit      — разряд из регистра
  uint32_t level = (*gpio_in >> led_pin()) & 1u;
  printf("  %-13s %u\n", "led bit", level);

  //   gpio_get     — то же значение через SDK
  printf("  %-13s %u\n", "gpio_get", gpio_get(led_pin()));
}
