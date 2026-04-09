#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

#include <Arduino.h>

// Intentionally disabled in this repo until the new control-board firmware
// baseline is cleaned up and secrets are provided through a safer path.
constexpr bool CAPTAIN_OTA_ENABLED = false;
constexpr const char* CAPTAIN_WIFI_SSID = "YOUR_WIFI_SSID";
constexpr const char* CAPTAIN_WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
constexpr const char* CAPTAIN_OTA_HOSTNAME = "captain-control-bringup";
constexpr const char* CAPTAIN_OTA_PASSWORD = "CHANGE_ME";
constexpr uint32_t CAPTAIN_WIFI_CONNECT_TIMEOUT_MS = 15000;

#endif
