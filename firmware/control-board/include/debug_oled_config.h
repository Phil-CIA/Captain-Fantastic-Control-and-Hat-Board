#ifndef DEBUG_OLED_CONFIG_H
#define DEBUG_OLED_CONFIG_H

#include <stdint.h>

// Debug/troubleshooting OLED added to the control board for boot messages
// and bring-up status. This is now identified as a common .96-inch
// SSD1306-style I2C module.
constexpr bool CAPTAIN_DEBUG_OLED_ENABLED = true;
constexpr uint8_t CAPTAIN_DEBUG_OLED_I2C_ADDRESS_PRIMARY = 0x3C;
constexpr uint8_t CAPTAIN_DEBUG_OLED_I2C_ADDRESS_SECONDARY = 0x3D;
constexpr uint16_t CAPTAIN_DEBUG_OLED_WIDTH = 128;
constexpr uint16_t CAPTAIN_DEBUG_OLED_HEIGHT = 64;
constexpr int8_t CAPTAIN_DEBUG_OLED_RESET_PIN = -1;

#endif
