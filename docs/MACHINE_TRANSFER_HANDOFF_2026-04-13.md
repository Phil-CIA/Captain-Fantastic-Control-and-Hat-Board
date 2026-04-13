# Machine Transfer Handoff (2026-04-13)

Purpose: resume firmware and bench bring-up on another machine without losing context.

## Current Hardware State
- Control board and HAT board are assembled.
- ESP is powered and currently running legacy firmware from the older combined-board setup.
- HAT board has not been respun yet.
- Current control-board schematic in use (legacy MOSFET path):
  - `C:\Users\forch\OneDrive\KiCad projects\pinball board\Pinball matrix board\Pinball Control board\Pinball Control board.kicad_sch`
- Legacy control board includes `IRFR5410` and `BSS84` devices.
- This is not the new TPS1H200A board revision (that revision is on the way).

## Confirmed Safety/Power Plan
- First validation remains 5V-only, current-limited bench supply.
- Keep real `+22vdc` / `26V` solenoid rail disconnected during first bring-up checks.
- Do not use USB as the first and only fault-protection power source.
- After safe first-pass validation, USB can be used for normal firmware/programming convenience.

## What Was Corrected in Session
- Removed TPS1H200A-focused test assumptions for the currently assembled legacy board.
- Re-aligned bring-up flow to legacy hardware realities and HAT-first testing priority.

## Immediate Goal on Other Machine
- Set up VS Code + PlatformIO and establish reliable firmware upload/monitor path.
- Capture baseline boot behavior before making firmware changes.

## First Actions (Other Machine)
1. Open this repo in VS Code.
2. Ensure PlatformIO extension is installed.
3. Connect board by USB (only after initial safe-power checks are satisfied).
4. Identify serial port and board environment in `platformio.ini`.
5. Build firmware.
6. Upload firmware.
7. Open serial monitor and capture full boot log.
8. Verify no reset loop/brownout/watchdog events.

## Baseline Validation Checklist
- Record supply conditions used for test (voltage/current limit).
- Record idle current draw.
- Record measured rails (`5V`, `3V3`) at startup and steady state.
- Verify at least one known input path and one known output path across split boards.
- Note any pin-map mismatch versus legacy combined-board assumptions.

## If Chat History Is Missing On Other Machine
Use this as first prompt in Copilot Chat:

"Resuming pinball control + HAT bring-up. Current boards are assembled legacy MOSFET design (IRFR5410/BSS84), not TPS1H200A. Keep 22-26V rail disconnected, do 5V current-limited logic bring-up first, then continue PlatformIO setup, build/upload, and serial baseline capture. Proceed step-by-step and run commands directly."

## Notes
- Related planning docs:
  - `docs/FIRST_POWERUP_CHECKLIST.md`
  - `docs/NEXT_STEP_BENCH_PLAN.md` (contains TPS1H200A items that are for the new revision)
  - `docs/FIRMWARE_MIGRATION.md`
