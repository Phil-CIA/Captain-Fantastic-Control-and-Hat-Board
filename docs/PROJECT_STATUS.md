# Project Status

## Summary
This repo tracks the **Captain Fantastic base control board** and its companion **HAT board** as a focused hardware project.

## Current state (2026-04-11)
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
- The repeated smart-switch channels have now been duplicated into the control-board schematic and saved to GitHub

## Immediate next priorities
1. use `docs/NEXT_STEP_BENCH_PLAN.md` and `docs/FIRST_POWERUP_CHECKLIST.md` for the next safe bench step
2. do the unpowered meter checks and the first **current-limited 5 V only** power-up
3. validate **one `TPS1H200A-Q1` channel at a time** before using the real coil rail
4. use `docs/TPS1H200_CL_DELAY_BENCH_TABLE.md` to log `CL` and `DELAY` behavior while tuning the first accepted default population
5. carry the repeated smart-switch pattern into final schematic / PCB cleanup once bench behavior is confirmed

## Implementation checkpoint
- The project now has a concrete implementation note, a CL/DELAY bench-tuning table, a schematic conversion worksheet, and a simple next-step bench plan.
- The repo-local KiCad libraries for `TPS1H200A-Q1` are now present under `hardware/control-board/kicad-lib/`.
- The repeated `TPS1H200A-Q1` channels for the solenoid outputs are now in the control-board schematic and the repo is saved cleanly.

## Open questions
- should the next revision keep the split **base board + HAT board** architecture, or fold more back into one board?
- is the best short-term solenoid workaround a `Vgs` clamp / gate-drive adjustment, or bypassing the on-board driver during development?
- what supporting firmware or connector documentation should move into this repo later?
