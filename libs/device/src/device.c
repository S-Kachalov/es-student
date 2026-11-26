#include "device/device.h"
#include "firmware.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include "pico/unique_id.h"
#include "pico/version.h"
#include <stddef.h>

void device_get_info(device_info_t *info) {
  info->name = FIRMWARE_NAME;
  info->version = FIRMWARE_VERSION;
  info->project = FIRMWARE_PROJECT;
  info->repo = FIRMWARE_REPO;
  info->board = DEVICE_BOARD;
  pico_get_unique_board_id_string(info->serial, sizeof(info->serial));
  volatile uint32_t *chip_id =
      (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
  uint32_t id = *chip_id;
  info->manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >>
                       SYSINFO_CHIP_ID_MANUFACTURER_LSB;
  info->part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
  info->revision =
      (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;
  info->sdk_version = PICO_SDK_VERSION_STRING;
}
