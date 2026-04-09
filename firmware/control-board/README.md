# Control-Board Firmware

This folder is the clean firmware starting point for the **Captain Fantastic base control board** in this repo.

## Purpose
- keep a **clean bring-up-oriented `main.cpp`** for the new board work
- preserve the older control firmware as a **reference import** for merging features back in gradually
- support the current development plan: **build the full board, power both rails at 5 V for now if needed, and keep the real 26 V solenoid supply out of service**

## Layout
- `src/main.cpp` – new clean bring-up main for firmware development
- `src/display/displays.cpp` – migrated 7-segment display helper
- `include/` – copied configuration and protocol headers needed for the control-board codebase
- `reference/legacy_control_main.cpp` – previous larger control firmware kept only as a source to merge from

## Bring-up intent
The new `main.cpp` is intentionally minimal:
- boots the MCU
- initializes I2C and the score display
- scans the I2C bus and brings up the added **.96-inch SSD1306-style OLED** for local boot/status text when present
- keeps the solenoid outputs in a safe default state
- allows optional low-voltage output proof-of-life testing only when intentionally enabled

## Next firmware steps
1. verify the clean bring-up build on the target control board
2. move over only the working gameplay / display / input pieces from `reference/legacy_control_main.cpp`
3. keep hardware-specific experiments and dead ends out of the new `main.cpp`
4. add the revised board pin or feature changes as they are confirmed on hardware
