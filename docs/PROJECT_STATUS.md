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
- The repeated smart-switch channels are now in the control-board schematic
- A routed control-board PCB checkpoint now exists in `hardware/control-board/Pinball Control board.kicad_pcb`
- The main `TPS1H200A-Q1` footprint-library mismatch has now been corrected in both the schematic and PCB files

## Immediate next priorities
1. use `docs/PCB_ROUTE_AND_ORDER_CHECKLIST.md` as the final review guide for the routed board
2. run the final KiCad **DRC** and do the last connector / mechanical sanity check
3. generate the manufacturing output set: **Gerbers** and **drill files**
4. if assembly support is needed, export the **BOM** and **pick-and-place** files too
5. order the revised board, then return to the bench bring-up plan when the hardware arrives

## Implementation checkpoint
- The project now has a concrete implementation note, a CL/DELAY bench-tuning table, a schematic conversion worksheet, a simple next-step bench plan, and a routed-board order checklist.
- The repo-local KiCad libraries for `TPS1H200A-Q1` are now present under `hardware/control-board/kicad-lib/`.
- The repeated `TPS1H200A-Q1` channels for the solenoid outputs are now in both the schematic and the routed PCB checkpoint.

## Open questions
- should the next revision keep the split **base board + HAT board** architecture, or fold more back into one board?
- is the best short-term solenoid workaround a `Vgs` clamp / gate-drive adjustment, or bypassing the on-board driver during development?
- what supporting firmware or connector documentation should move into this repo later?
