#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

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
