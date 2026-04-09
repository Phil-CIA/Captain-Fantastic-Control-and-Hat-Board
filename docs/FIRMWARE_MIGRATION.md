# Firmware Migration Notes

This file tracks the move from the older mixed-purpose Captain Fantastic repo into the new focused control-board / HAT-board repo.

## Current migration state
A clean control-board firmware baseline has now been added under:
- `firmware/control-board/`

### Imported pieces
- `src/display/displays.cpp` – migrated display helper from the older repo
- `include/` – copied protocol and hardware-config headers needed for ongoing work
- `reference/legacy_control_main.cpp` – previous larger control firmware kept as a reference source for selective merging

### New clean starting point
- `src/main.cpp` is a new bring-up-oriented firmware entry point for this repo
- `platformio.ini` is self-contained and no longer points back to the old repository layout
- `include/ota_config.h` was intentionally sanitized so secrets are not carried into the new repo

## Current development intent
- keep the new active firmware clean and readable
- merge over only the working pieces from the old control firmware
- avoid carrying forward old experiment clutter unless it is still needed
- support the current bench plan: **build the whole board, keep the real 26 V rail out of service, and if needed feed both rails with current-limited 5 V for proof-of-life testing only**
- use the newly added **.96-inch SSD1306-style I2C OLED** as a local troubleshooting display for boot messages and status; the clean bring-up baseline now targets that controller family directly

## Likely next merge targets
1. direct input handling
2. display updates and score output
3. matrix-board I2C communication helpers
4. headbox lamp control
5. audio / OTA only after the new baseline is stable

## Notes
- The new `main.cpp` is intended to become the long-term source of truth.
- The legacy file is reference material, not the place to keep adding new complexity.
