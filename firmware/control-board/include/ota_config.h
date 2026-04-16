#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

#include <Arduino.h>

// Intentionally disabled in this repo until the new control-board firmware
// baseline is cleaned up and secrets are provided through a safer path.
constexpr bool CAPTAIN_OTA_ENABLED = false;
constexpr const char* CAPTAIN_WIFI_SSID = "Forche main 2.4";
constexpr const char* CAPTAIN_WIFI_PASSWORD = "gizmoa22";
constexpr const char* CAPTAIN_OTA_HOSTNAME = "captain-control-bringup";
constexpr const char* CAPTAIN_OTA_PASSWORD = "CHANGE_ME";
constexpr uint32_t CAPTAIN_WIFI_CONNECT_TIMEOUT_MS = 15000;

// Optional staged OTA defaults for no-argument flashing.
// Example: "http://192.168.1.50:8000/firmware.bin"
constexpr const char* CAPTAIN_OTA_DEFAULT_URL = "http://192.168.0.60:8000/firmware.bin";

// If true and CAPTAIN_OTA_DEFAULT_URL is set, device starts OTA download on boot.
constexpr bool CAPTAIN_OTA_AUTOSTART_ON_BOOT = false;

#endif
