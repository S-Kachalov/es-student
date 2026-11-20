#pragma once
#include <stdint.h>

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.0.0"

#define DEVICE_PROJECT "211-command-usb"
#define DEVICE_REPO "https://github.com/S-Kachalov/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

extern struct info_t device_card;

typedef struct info_t {
  uint32_t version;
  char name[13];
  uint8_t revision;
} info_t;

void device_info(void);
void dev_info(void);
