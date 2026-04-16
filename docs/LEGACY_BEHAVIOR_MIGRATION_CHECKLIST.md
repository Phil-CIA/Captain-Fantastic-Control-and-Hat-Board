# Legacy Behavior Migration Checklist

This checklist captures the old Captain Fantastic Home Edition control behavior that should be preserved while moving onto the dedicated control-board + HAT-board hardware.

## Preserve unchanged machine behavior

- Boot with all solenoid outputs forced safe/OFF before higher-level logic runs.
- Bring up the score display as HT16K33 at `0x70` and keep the same score semantics.
- Keep machine startup flow: safe boot, display bring-up, audio startup indication, attract-ready state.
- Keep switch meaning the same even if the physical routing changes.
- Keep solenoid meaning and pulse semantics the same even if the driver hardware changes.
- Keep diagnostic mode conceptually aligned with the legacy Bally-style sequence.
- Preserve event-audio intent: startup sound, switch/solenoid/game-state sounds, and eventual music playback.

## Legacy display behavior to preserve

- Single HT16K33 backpack at `0x70`.
- 6 usable digits with the legacy position mapping.
- Startup display self-test before normal score updates.
- Score clamped to `0..999999`.

Primary references:

- `Captain-Fantastic-home-edition/src/displays.cpp`
- `Captain-Fantastic-home-edition/include/displays.h`

## Legacy audio behavior to preserve

- Audible startup confirmation on boot.
- Event-driven sound effects for gameplay actions.
- Support for stored music playback from flash/storage.
- Keep audio output under software control for safe bench volume tuning.

Primary references:

- `Captain-Fantastic-home-edition/src/main_firmware.cpp`
- `Captain-Fantastic-home-edition/docs/MP3_MUSIC_SETUP.md`
- `Captain-Fantastic-home-edition/music_server.py`

## Legacy switch/solenoid behavior to preserve

- Debounced switch handling.
- Same scoring intent for lanes, bumpers, targets, return lanes, bonus lanes, slingshots, start, tilt, and test.
- Same machine-level consequences: score changes, solenoid pulse requests, game-state transitions.

Primary references:

- `Captain-Fantastic-home-edition/src/main_firmware.cpp`
- `Captain-Fantastic-home-edition/src/switches.cpp`
- `Captain-Fantastic-home-edition/src/Solenoid.cpp`

## New repo implementation sequence

1. Establish safe bring-up hardware behavior.
2. Restore display parity.
3. Restore audio parity.
4. Restore direct-input and gameplay-event parity.
5. Restore solenoid command semantics on the new hardware.
6. Rebuild diagnostic/test-mode parity.

## Immediate current checkpoint

- OLED debug path works.
- MAX98357 I2S path works.
- Startup test tones work.
- Startup MP3 playback path is now scaffolded for SPIFFS files in `firmware/control-board/data/`.