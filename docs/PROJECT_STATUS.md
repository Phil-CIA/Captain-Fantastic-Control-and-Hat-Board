# Project Status

## Summary
This repo tracks the **Captain Fantastic control board** and the related **HAT board** redesign work as a focused hardware project.

## Current state (2026-04-09)
### Hardware
- The current KiCad files uploaded to this repo are now the authoritative baseline for both boards
- `hardware/control-board/` contains the working control-board source for the next redesign pass
- `hardware/control-board/archive/` keeps older control-board snapshots / backup exports for reference
- `hardware/hat-board/` contains the current / reference HAT board source
- The recently received board and parts exposed **at least two issues serious enough to require redesign**

### Repo state
- New focused repo has been created to keep this work separate from the matrix board and other machine firmware
- Root-level upload clutter is being folded into the organized `hardware/` tree so the baseline stays clear

## Immediate next priorities
1. document the two known hardware issues clearly
2. review the current control board and HAT board files against the physical boards received
3. mark which files are authoritative and which are only historical reference
4. prepare the redesign plan for the next board revision

## Open questions
- what exactly are the first two must-fix redesign issues?
- does the HAT board remain part of the active architecture or only a reference design?
- what supporting firmware or connector documentation should move into this repo later?
