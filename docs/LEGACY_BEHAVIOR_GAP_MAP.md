# Legacy Behavior Gap Map

This maps the old Home Edition control behavior against the current dedicated control-board bring-up firmware.

## Already present in the new repo

- Safe boot intent and solenoid-safe defaults.
- HT16K33 display probe and startup test path.
- OLED debug display for local bring-up feedback.
- MAX98357 I2S audio hardware bring-up.
- Simple audible boot confirmation.
- Direct-input polling scaffold.

Files:

- `firmware/control-board/src/main.cpp`
- `firmware/control-board/include/audio_i2s_config.h`
- `firmware/control-board/include/direct_input_config.h`
- `firmware/control-board/include/solenoid_gpio_config.h`

## Partially restored

- Audio playback:
  - Startup tone path is working.
  - Minimal SPIFFS startup MP3 playback path is added, but depends on loading files into `firmware/control-board/data/` and uploading SPIFFS.

- Display behavior:
  - Basic score/test path exists.
  - Full gameplay-driven score ownership is not yet restored.

## Missing from the new repo versus old behavior

- Full game-state machine parity.
- Legacy switch scoring map and event logic.
- Solenoid/event coupling parity.
- Full Bally-style diagnostic mode parity.
- Full attract-mode behavior parity.
- OTA/web-update parity from the older repo.
- Rich music system parity beyond minimal boot-file playback.

## Best source files for future migration

- `Captain-Fantastic-home-edition/src/main_firmware.cpp`
- `Captain-Fantastic-home-edition/src/displays.cpp`
- `Captain-Fantastic-home-edition/src/switches.cpp`
- `Captain-Fantastic-home-edition/src/Solenoid.cpp`
- `Captain-Fantastic-home-edition/docs/DIAGNOSTIC_TEST_MODE.md`
- `Captain-Fantastic-home-edition/docs/MP3_MUSIC_SETUP.md`
- `Captain-Fantastic-home-edition/Captain-v2/src/control_main.cpp`