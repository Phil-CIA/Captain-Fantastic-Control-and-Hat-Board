# Control-Board Firmware Modes and Split (2026-04-16)

## Why this split exists
The control-board firmware now supports two profiles from one codebase:
- `SYSTEM` profile: default runtime behavior for integrated machine operation.
- `TEST` profile: hardware validation mode with extra diagnostics and optional output test behavior.

This keeps production behavior stable while preserving fast bring-up workflows.

## Build environments
Defined in `platformio.ini`:
- `captain_control_system` (default)
- `captain_control_test`
- `captain_control_bringup` (legacy compatibility)

## Profile flag contract
Compile-time flag:
- `CAPTAIN_APP_MODE_TEST=0` -> `SYSTEM`
- `CAPTAIN_APP_MODE_TEST=1` -> `TEST`

Header:
- `include/app_mode_config.h`

Main profile symbols:
- `CAPTAIN_APP_MODE_IS_TEST`
- `CAPTAIN_APP_MODE_NAME`
- `CAPTAIN_ENABLE_OUTPUT_TEST_DEFAULT`
- `CAPTAIN_ENABLE_DEEP_INPUT_OVERLAY`

## Current module boundaries
- `src/main.cpp`: integration runtime/orchestrator (boot, loop, OLED status, headbox routing, command surface).
- `src/command/command_runtime.cpp`: single serial polling entrypoint and command dispatch.
- `src/command/input_commands.cpp`: direct input command handlers (`input status`).
- `src/command/ota_commands.cpp`: OTA command handlers (`ota help/status/wifi/flash`).
- `src/command/audio_commands.cpp`: audio command handlers (`mp3 ...`).
- `src/display/displays.cpp`: 7-segment rendering and message helpers.
- `src/input/direct_input_runtime.cpp`: direct input pin setup, polling cadence, edge detection, and asserted-state cache.
- `src/input/input_overlay_runtime.cpp`: input-state overlay formatting plus idle/event display trigger policy.
- `src/ota/ota_runtime.cpp`: OTA state machine, staged download/apply flow, and Wi-Fi credential/reconnect maintenance.
- `src/audio/audio_runtime.cpp`: MP3 storage init, event-file selection, playback loop, and I2S diagnostic fallback.
- `include/*.h`: hardware mappings and subsystem config.

## Behavior differences by profile
### SYSTEM profile
- Output test defaults OFF.
- Input event OLED overlays are prioritized for `Tilt` and `Start` transitions.
- Suitable for full-system integration testing.

### TEST profile
- Output test defaults ON via profile flag.
- Input event overlays are shown for all direct inputs.
- Suitable for bench diagnostics and wiring validation.

## Runtime command additions
- `mode` prints active profile and key mode toggles.
- `input status` prints and displays current SW1/SW2/Tilt/Start states.

## Next split steps (recommended)
1. Separate test-only command handlers from shared command parsing.
2. Keep `src/main.cpp` as orchestrator only and avoid subsystem-local state growth.
3. Trim stale includes/constants from orchestrator files after each completed split.
