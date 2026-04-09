# Project Status

## Summary
This repo tracks the **Captain Fantastic base control board** and its companion **HAT board** as a focused hardware project.

## Current state (2026-04-09)
### Hardware
- The current KiCad files uploaded to this repo are now the authoritative baseline for both boards
- The machine originally had two main boards: the matrix board and the control board
- During development, the control board was split into a **base control board** plus a **HAT board**
- The HAT board is a companion board from that split, **not** a revision of the control board
- `hardware/control-board/` contains the working base-board source for the next redesign pass
- `hardware/control-board/archive/` keeps older control-board snapshots / backup exports for reference
- `hardware/hat-board/` contains the companion HAT-board source
- The recently received board and parts exposed **two issues serious enough to require redesign**

### Repo state
- New focused repo has been created to keep this work separate from the matrix board and other machine firmware
- Root-level upload clutter is being folded into the organized `hardware/` tree so the baseline stays clear
- The immediate goal is **continued board build-out and development**, so some issues may be handled with short-term workarounds before the full PCB respin is finalized

## Immediate next priorities
1. define a safe workaround for the **26 V solenoid-drive problem**, where the P-channel MOSFET driver arrangement is pushing `Vgs` above the recommended range
2. continue development using the current **HAT-board connector workaround**, while keeping the physical alignment fix on the Rev B PCB list
3. work through the task breakdown in `docs/REV_B_ACTION_LIST.md`
4. prepare the redesign plan for the next board revision

## Open questions
- should the next revision keep the split **base board + HAT board** architecture, or fold more back into one board?
- is the best short-term solenoid workaround a `Vgs` clamp / gate-drive adjustment, or bypassing the on-board driver during development?
- what supporting firmware or connector documentation should move into this repo later?
