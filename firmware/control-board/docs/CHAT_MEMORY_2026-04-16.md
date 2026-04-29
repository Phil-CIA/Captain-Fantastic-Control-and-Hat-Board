# Chat Memory - 2026-04-16

## Session goals completed
- Added headbox LED routing back into active control-board runtime.
- Added staged OTA visual telemetry to local displays.
- Added OLED Wi-Fi icon and OTA progress bar.
- Added direct input diagnostics and `input status` command for SW1/SW2/Tilt/Start.
- Corrected direct input polarity to match hardware pull-down + switch-to-3V3 wiring.
- Added boot Wi-Fi connection and periodic reconnect maintenance.
- Introduced SYSTEM vs TEST profile split scaffolding.
- Extracted direct-input polling/state handling into `src/input/direct_input_runtime.cpp`.
- Extracted input overlay formatting/timing triggers into `src/input/input_overlay_runtime.cpp`.
- Extracted serial command parsing/routing into `src/command/command_runtime.cpp`.
- Split command handlers by domain into `src/command/input_commands.cpp`, `src/command/ota_commands.cpp`, and `src/command/audio_commands.cpp`.
- Extracted OTA + Wi-Fi maintenance flow into `src/ota/ota_runtime.cpp`.
- Extracted MP3/I2S/storage playback flow into `src/audio/audio_runtime.cpp`.

## Key hardware assumptions used in firmware
- Direct input test switches are wired to +3V3 through switch contacts with pull-down resistors to GND.
- Therefore direct inputs are treated as active-high in current config.
- OLED is SSD1306 at 0x3C/0x3D candidate addresses.
- External flash W25Q128 is the OTA stage storage and preferred MP3 source.

## Current OTA runtime model
1. Connect Wi-Fi (or reconnect if dropped).
2. Download OTA image over HTTP(S) to external flash stage file.
3. Flash staged image into internal app partition.
4. Reboot on success.

## Display model snapshot
- 7-segment: score and short status words.
- OLED: richer state text, Wi-Fi status icon, OTA progress bar, and input overlays.

## Commands in active firmware
- `mp3 help`
- `ota help`
- `ota status`
- `ota wifi <ssid> <password>`
- `ota flash [url]`
- `input status`
- `mode`

## Open follow-up items
- Separate test-only command surface from system profile command surface.
- Add dedicated audio content review doc (event -> file mapping, gain, loop policy, fallback behavior).

## Daily update - 2026-04-29

### Firmware identity and traceability
- Added firmware identity line at boot with version, build ID, checksum signature, and compile timestamp.
- Build ID now comes from PlatformIO environment and UNIX time.
- Serial confirmation captured on hardware:
	- `FW: control-board | build=captain_control_test-1777487289 | sig=17D9D395 | built=Apr 29 2026 ...`

### Upload/port reliability updates
- Confirmed `COM4` upload path for matrix/display board using `display_board` environment.
- Confirmed `COM5` upload path for control board.
- Added manual-boot-safe upload flags in `platformio.ini` (`--before no_reset`, `--after hard_reset`).

### Logging quality improvements
- Added per-switch debounce suppression and per-window rate limiting for matrix switch logs.
- Added compact 5-second summary line with logged/suppressed totals and top noisy switch.

### Output-test behavior update
- In TEST profile, output exercise now runs even when matrix link is degraded.
- Verified runtime output cycling on control board (`S2`..`S6`) via serial monitor.

### Tomorrow focus
- Connect 5V solenoid-LED supply and validate commanded playfield-light behavior from control board.
- Keep 26V rail disconnected during initial morning tests.
