# Control-Board Firmware

This folder is the clean firmware starting point for the **Captain Fantastic base control board** in this repo.

## Purpose
- keep a **clean bring-up-oriented `main.cpp`** for the new board work
- preserve the older control firmware as a **reference import** for merging features back in gradually
- support the current development plan: **build the full board, power both rails at 5 V for now if needed, and keep the real 26 V solenoid supply out of service**

## Layout
- `src/main.cpp` – integration orchestrator for runtime subsystems
- `src/command/command_runtime.cpp` – serial polling entrypoint and command dispatch
- `src/command/input_commands.cpp` – direct input command handlers
- `src/command/ota_commands.cpp` – OTA command handlers
- `src/command/audio_commands.cpp` – audio command handlers
- `src/display/displays.cpp` – migrated 7-segment display helper
- `src/input/direct_input_runtime.cpp` – direct input polling and state cache runtime
- `src/input/input_overlay_runtime.cpp` – direct-input overlay formatting, idle refresh, and event/status trigger glue
- `src/ota/ota_runtime.cpp` – staged OTA + Wi-Fi maintenance runtime
- `src/audio/audio_runtime.cpp` – MP3 playback, storage sync, and I2S fallback runtime
- `include/` – copied configuration and protocol headers needed for the control-board codebase
- `reference/legacy_control_main.cpp` – previous larger control firmware kept only as a source to merge from
- `docs/FIRMWARE_MODES_AND_SPLIT_2026-04-16.md` – system/test profile split contract
- `docs/CHAT_MEMORY_2026-04-16.md` – session memory and decisions captured in-repo

## Build profiles
The firmware now supports profile-based builds:
- `captain_control_system` (default): integration-safe behavior
- `captain_control_test`: test-heavy behavior and deeper diagnostics
- `captain_control_bringup`: retained for compatibility while migration continues

Profile selection is controlled by `CAPTAIN_APP_MODE_TEST` via `include/app_mode_config.h`.

Quick commands:
1. `platformio run -e captain_control_system`
2. `platformio run -e captain_control_test`

## Bring-up intent
The new `main.cpp` is intentionally minimal:
- boots the MCU
- initializes I2C and the score display
- scans the I2C bus and brings up the added **.96-inch SSD1306-style OLED** for local boot/status text when present
- keeps the solenoid outputs in a safe default state
- allows optional low-voltage output proof-of-life testing only when intentionally enabled

## Next firmware steps
1. separate test-only command behavior from shared command handlers while preserving one serial entrypoint
2. move over only the working gameplay / display / input pieces from `reference/legacy_control_main.cpp`
3. keep hardware-specific experiments and dead ends out of the system profile path
4. add the revised board pin or feature changes as they are confirmed on hardware
