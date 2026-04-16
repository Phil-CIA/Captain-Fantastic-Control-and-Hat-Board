#ifndef APP_MODE_CONFIG_H
#define APP_MODE_CONFIG_H

#ifndef CAPTAIN_APP_MODE_TEST
#define CAPTAIN_APP_MODE_TEST 0
#endif

constexpr bool CAPTAIN_APP_MODE_IS_TEST = (CAPTAIN_APP_MODE_TEST != 0);
constexpr const char* CAPTAIN_APP_MODE_NAME = CAPTAIN_APP_MODE_IS_TEST ? "TEST" : "SYSTEM";

// Test profile can pulse outputs and keep a denser diagnostic overlay.
constexpr bool CAPTAIN_ENABLE_OUTPUT_TEST_DEFAULT = CAPTAIN_APP_MODE_IS_TEST;
constexpr bool CAPTAIN_ENABLE_DEEP_INPUT_OVERLAY = CAPTAIN_APP_MODE_IS_TEST;

#endif