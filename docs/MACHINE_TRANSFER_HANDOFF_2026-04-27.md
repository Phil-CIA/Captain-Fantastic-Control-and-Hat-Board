# Machine Transfer Handoff (2026-04-27)

Purpose: continue matrix-board bring-up work from a different development station without losing current direction.

## Session Summary
- We confirmed this local workspace is the control-board/HAT repo, not the active matrix hardware repo.
- You confirmed the active target is the separate matrix repo:
  - https://github.com/Phil-CIA/Pinball-Sw-Lamp-Matrix-
- We pulled the current matrix repo status from remote docs and aligned to the current bench stage:
  - board state = assembled, not powered yet
  - project phase = first bring-up / hardware validation

## Matrix Repo Status Snapshot (for restart)
- Architecture baseline remains:
  - 8x4 lamp matrix driver + 8x4 switch scanner
  - VNQ7E100AJTR high-side row drivers
  - LMV393 comparator switch readback
  - 74HC595 output expansion
  - ESP32-C6 firmware bring-up scaffold (ESP-IDF via PlatformIO)
- Documented project state indicates Rev 1 hardware ordering and bring-up-prep workflow.
- Bring-up docs already define staged flow:
  - visual inspection
  - continuity checks (18V/5V/3.3V to GND)
  - current-limited first power
  - logic validation
  - one-row/one-column lamp validation before full scan

## Agreed Restart Point
- Latest matrix board is assembled and unpowered.
- Immediate objective: safe first power-up and logic-path validation before broad lamp stress testing.

## Execution Plan on Dev Station
1. Session prep and stop conditions
   - Bench supply current limit set before power.
   - DMM + scope/logic probe ready.
   - Hard stop rules defined (overcurrent, wrong rail, unusual heating/smell).

2. Unpowered checks
   - Visual inspection (orientation, solder defects, connector pin 1).
   - Rail continuity/short checks:
     - 18V to GND
     - 5V to GND
     - 3.3V to GND
   - Verify no accidental coupling from lamp rail into logic rails.

3. First power-up (current-limited)
   - Bring up supply conservatively.
   - Verify 5V rail first, then 3.3V logic rail.
   - Confirm no abnormal heating on power/protection/driver areas.

4. Firmware/logic smoke test
   - Run existing ESP32-C6 bring-up firmware.
   - Verify boot output.
   - Probe shift-register lines: SR_SCLK, SR_LATCH, SR_DATA0.
   - Validate switch-column readback behavior: SW_COL_0..SW_COL_3.

5. Boot safety checks
   - Verify /OE and /MR behavior is safe at startup.
   - Confirm no unintended lamp flash at boot.

6. Staged lamp-path validation
   - Test one row + one column first.
   - Expand to supervised full scan only after first pass is stable.
   - Watch connector temperature, supply sag, and thermal hotspots.

7. Capture findings immediately
   - Log rail values, current draw, thermal observations, and mapping/polarity notes.
   - Update matrix repo docs with findings before ending session.

## Files To Use First (matrix repo)
- README.md
- docs/PROJECT_STATUS.md
- docs/BRINGUP_CHECKLIST.md
- docs/NEXT_ITERATION.md
- docs/PINMAP.md
- firmware/src/main.cpp

## Suggested First Prompt on Dev Station
"Resuming matrix-board bring-up on assembled unpowered Rev 1 hardware. Use docs/BRINGUP_CHECKLIST.md and docs/NEXT_ITERATION.md as primary flow. Execute staged checks: unpowered continuity, current-limited first power, logic probe of SR_SCLK/SR_LATCH/SR_DATA0 and SW_COL_0..SW_COL_3, then one-row/one-column lamp test before full scan. Record pass/fail and measured values after each phase."

## Scope Reminder
- This handoff captures matrix-board bring-up direction only.
- Control-board/HAT repo changes were not the target for this restart session.
