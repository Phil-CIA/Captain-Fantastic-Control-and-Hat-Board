# Project Status

## Summary
This repo tracks the **Captain Fantastic control board** and the related **HAT board** redesign work as a focused hardware project.

## Current state (2026-04-09)
### Hardware
- Baseline KiCad files were copied in from the larger `Captain-Fantastic-home-edition` repo
- `hardware/control-board/` contains the current control board source
- `hardware/hat-board/` contains the legacy / reference HAT board source
- The recently received board and parts exposed **at least two issues serious enough to require redesign**

### Repo state
- New focused repo has been created to keep this work separate from the matrix board and other machine firmware
- Cleanup is still needed to decide what should remain in the repo versus what should stay archived elsewhere

## Immediate next priorities
1. document the two known hardware issues clearly
2. review the current control board and HAT board files against the physical boards received
3. mark which files are authoritative and which are only historical reference
4. prepare the redesign plan for the next board revision

## Open questions
- what exactly are the first two must-fix redesign issues?
- does the HAT board remain part of the active architecture or only a reference design?
- what supporting firmware or connector documentation should move into this repo later?
