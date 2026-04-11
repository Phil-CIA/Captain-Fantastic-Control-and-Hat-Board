# PCB Route and Order Checklist

This is the **current follow-through checklist** now that the revised control-board PCB has been routed.

## Current checkpoint (2026-04-11)
- the revised control-board PCB has now been routed in KiCad
- the `TPS1H200A-Q1` devices (`U1`..`U5`) are now on the PCB
- the main `TPS1H200A-Q1` footprint-library mismatch has been corrected so those devices now point to the repo-local footprint library
- current file checks report **no errors found** for the schematic and PCB files

## Remaining library note
The main smart-switch footprint issue is fixed.

A few older project-specific footprints still use these external library names:
- `1My_Parts_ Passive:Slide Sw LCSC#C7498220` — 2 references
- `ESP32Vroom:CONN_SDA-41792_M450_23_MOL` — 1 reference

These are **not expected to block Gerber / drill export**, because the footprint geometry is already stored in the `.kicad_pcb` file.

If KiCad warns about them on another machine, the practical choices are:
1. keep using the current board as-is for fabrication output, or
2. later copy those legacy footprints into a repo-local library for full portability.

## Final pre-order checklist

### 1) KiCad review
- reopen the PCB and confirm the board still looks correct after the latest save
- run **DRC** in KiCad
- confirm there are no unexpected unrouted nets or accidental shorts

### 2) Mechanical / placement review
- verify connector orientation and pin numbering
- verify mounting holes and board outline
- verify silkscreen labels are readable and not covering pads
- verify polarity-sensitive parts still point the correct way

### 3) Power-path review
- re-check the `+22vdc` feed and `GND` return routing
- confirm the `TPS1H200A-Q1` channels have sensible copper around them for current and heat spreading
- confirm the output paths to `S2`..`S6` look direct and consistent

### 4) Fabrication settings
- confirm the intended board stackup is still what you want
- current KiCad board file shows a standard-looking `1.6 mm` board with `0.035 mm` copper layers
- verify any fab defaults (mask, copper weight, finish, color) before ordering

### 5) Manufacturing outputs
- generate **Gerbers**
- generate **drill files**
- if assembly is planned later, also export **BOM** and **pick-and-place / position** files
- zip the manufacturing outputs for the board house order

## Stop and fix before ordering if you see
- any DRC errors that are real electrical or spacing problems
- any connector orientation doubt
- any too-thin high-current path on the `+22vdc` or output routing
- any missing or obviously wrong footprint on the revised smart-switch channels

## What happens after the order
- order the revised board
- when the boards arrive, return to `docs/NEXT_STEP_BENCH_PLAN.md` and `docs/FIRST_POWERUP_CHECKLIST.md`
- start with safe checks and controlled bring-up, one channel at a time
