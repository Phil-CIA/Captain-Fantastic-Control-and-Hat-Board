# Project Status

## Summary
This repo tracks the **Captain Fantastic base control board** and its companion **HAT board** as a focused hardware project.

## Current state (2026-04-12)
### Hardware
- The current KiCad files uploaded to this repo are now the authoritative baseline for both boards
- The machine originally had two main boards: the matrix board and the control board
- During development, the control board was split into a **base control board** plus a **HAT board**
- The HAT board is a companion board from that split, **not** a revision of the control board
- `hardware/control-board/` contains the working base-board source for the next redesign pass
- `hardware/control-board/archive/` keeps older control-board snapshots / backup exports for reference
- `hardware/hat-board/` contains the companion HAT-board source
- The solenoid-driver redesign direction is now centered on the **single-channel `TPS1H200A-Q1` smart high-side switch**

### Repo state
- New focused repo has been created to keep this work separate from the matrix board and other machine firmware
- Root-level upload clutter has been folded into the organized `hardware/` tree so the baseline stays clear
- The repo now contains project-local KiCad symbol / footprint / 3D-model support for `TPS1H200A-Q1`
- The repeated smart-switch channels are now in the control-board schematic
- A routed control-board PCB checkpoint now exists in `hardware/control-board/Pinball Control board.kicad_pcb`
- The main `TPS1H200A-Q1` footprint-library mismatch has now been corrected in both the schematic and PCB files

## Immediate next priorities
1. wait for delivery of the ordered Rev 1 PCB and BOM parts
2. use `docs/REV1_POWERUP_ON_ARRIVAL.md` as the arrival-session checklist
3. inspect received boards/parts and complete unpowered continuity checks first
4. perform first **current-limited 5 V only** bring-up before any real coil rail use
5. validate one smart-switch channel at a time and log CL/DELAY behavior

## Implementation checkpoint
- The project now has a concrete implementation note, a CL/DELAY bench-tuning table, a schematic conversion worksheet, a simple next-step bench plan, and a routed-board order checklist.
- The repo-local KiCad libraries for `TPS1H200A-Q1` are now present under `hardware/control-board/kicad-lib/`.
- The repeated `TPS1H200A-Q1` channels for the solenoid outputs are now in both the schematic and the routed PCB checkpoint.
- Rev 1 PCB and BOM orders are now placed; the project has moved into the delivery-and-bring-up stage.

## Open questions
- should the next revision keep the split **base board + HAT board** architecture, or fold more back into one board?
- is the best short-term solenoid workaround a `Vgs` clamp / gate-drive adjustment, or bypassing the on-board driver during development?
- what supporting firmware or connector documentation should move into this repo later?

## Firmware and bench checkpoint (2026-04-29)
### Confirmed this session
- Matrix/display board firmware upload on `COM4` completed successfully (`display_board` env).
- Control-board firmware upload on `COM5` completed successfully with serial boot confirmation.
- Firmware self-identification is now in boot logs:
	- `FW: <name> | build=<id> | sig=<hex> | built=<date time>`
- PlatformIO environment configuration now carries firmware ID macros and manual-boot-safe upload flags.
- Control-board switch logging now has suppression/rate-limiting summaries to reduce serial spam.
- TEST profile output exercise was validated on hardware with repeated output-test lines (`S2`..`S6`).

### Current practical state
- Control board can be flashed in either profile:
	- `captain_control_system` for normal integration behavior.
	- `captain_control_test` for active output cycling/bench proof-of-life.
- Matrix link can remain degraded while still running output test in TEST profile.
- Tomorrow's bench focus is to connect 5V for solenoid LEDs and validate playfield-light control path from control board.
