#ifndef DEBUG_OLED_CONFIG_H
#define DEBUG_OLED_CONFIG_H

#include <stdint.h>

// Debug/troubleshooting OLED added to the control board for boot messages
// and bring-up status. The exact controller model still needs to be confirmed
// before full text rendering support is enabled in firmware.
constexpr bool CAPTAIN_DEBUG_OLED_ENABLED = true;
constexpr uint8_t CAPTAIN_DEBUG_OLED_I2C_ADDRESS_PRIMARY = 0x3C;
constexpr uint8_t CAPTAIN_DEBUG_OLED_I2C_ADDRESS_SECONDARY = 0x3D;
constexpr uint16_t CAPTAIN_DEBUG_OLED_WIDTH = 256;
constexpr uint16_t CAPTAIN_DEBUG_OLED_HEIGHT = 128;

#endif
