# Single-Channel Solenoid Layout Plan

This note locks in the board-level direction for the next **base control-board** respin.

## Chosen direction
Use **single-channel smart high-side switch devices** for the five solenoid outputs instead of:
- the current discrete PMOS stage, or
- one larger multi-channel package as the primary layout strategy

## Why this was chosen
For this board, the layout has enough space and routing resources to support five individual channels cleanly.

That gives the following benefits:
- easier routing from each driver to its output connector
- easier thermal spreading
- easier debug and rework if one channel needs changes later
- easier physical placement around the existing control-board constraints

## Channel strategy
There are currently five named outputs in firmware mapping:
- `S2`
- `S3`
- `S4`
- `S5`
- `S6`

The layout plan is therefore:
- **one smart high-side channel per output**
- same channel pattern repeated five times
- keep each driver physically near the output path it serves where practical

## Per-channel block intent
Each output block should contain:
- one **single-channel smart high-side switch**
- local power decoupling close to the device
- output net to the solenoid connector
- logic input from the MCU / control logic
- optional fault or current-sense return if the chosen part supports it and routing allows it

Use `docs/SMART_SWITCH_CHANNEL_BLOCK.md` as the repeated per-channel schematic pattern for `S2..S6`.

## Placement guidance
- keep the high-current path short and direct from rail -> device -> output connector
- keep each channel's local decoupling close to the switch supply pins
- keep logic/control traces separated from the heavier output current path where practical
- give each device enough copper area for heat spreading
- avoid recreating the old discrete PMOS gate-drive cluster in the new layout

## Design note
The layout strategy is now locked to **single-channel `TPS1H200A-Q1`-based devices** because that is the easier and cleaner fit for this board.

A visible Rev B note has also been added directly into `hardware/control-board/Pinball Control board.kicad_sch` near the existing solenoid section so the KiCad source itself carries the direction during schematic/layout work.
