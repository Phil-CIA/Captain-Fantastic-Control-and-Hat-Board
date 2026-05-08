# Morning Start Handoff - 2026-05-08

Status: Ready for next-session execution.
Purpose: Start clean in the morning with one practical sequence that gets from bring-up toward playable behavior while keeping repo hygiene moving.

## Repositories In Scope
1. https://github.com/Phil-CIA/Captain-Fantastic-Control-and-Hat-Board
2. https://github.com/Phil-CIA/Pinball-Sw-Lamp-Matrix-
3. https://github.com/Phil-CIA/Captain-Fantastic-home-edition

## Current Snapshot

### Control-and-Hat-Board (main at d07bf7c)
- New handoff present: SOLENOID_BRINGUP_HANDOFF_2026-05-08.md
- Solenoid state:
  - S3/S4/S5/S6 working on TPS path
  - S2 (outhole) still insufficient force on TPS even with aggressive current-limit tuning
- Firmware currently in test profile behavior for focused S2 validation:
  - S2 pulse set to 100 ms
  - Single-channel repeated test mode enabled
- GPIO init-order fix applied to reduce boot glitch risk on GPIO5 path

### Pinball-Sw-Lamp-Matrix- (main at b2cbb86)
- Deterministic row scheduler and SR timing stabilization work already merged previously
- CI pin fix for espressif32 version already merged previously
- Latest commit is doc-only

### Captain-Fantastic-home-edition
- Not locally checked out in this workspace during this session
- Still part of integration scope as legacy behavior reference source

## Morning Start Plan (Execution Order)

1. Hardware path first: complete S2 outhole relay workaround bench wiring and validate force.
2. After S2 force is reliable, revert control test loop from single-channel S2 back to full five-channel cycle and confirm all five channels repeat cleanly.
3. Verify the four pending switch paths explicitly and log results:
   - Tilt
   - Maintenance/Test
   - Easy
   - Start
4. Define and lock operating mode behavior notes before deeper gameplay coding:
   - Boot
   - Test/Service
   - Attract
   - Gameplay
   - Degraded/Error
5. Begin gameplay coupling implementation once switch and mode behavior checks are captured:
   - Switch edge to solenoid action mapping
   - Initial score increment path
   - Lamp state update path

## Repo Cleanup Stream (Run In Parallel)

1. In each repo, create a dedicated handoff archive folder.
2. Move handoff/transfer session docs into that folder without renaming date-stamped files.
3. Add one chronological journey index per repo linking all handoffs in date order.
4. Refresh stale top-level status docs after technical state is validated.

## Immediate First Commands For Morning

1. Pull all repos on start:
   - Control-and-Hat-Board
   - Pinball-Sw-Lamp-Matrix-
   - Captain-Fantastic-home-edition
2. Build and flash control-board test profile to verify baseline before hardware edits.
3. Execute relay workaround validation checklist from SOLENOID_BRINGUP_HANDOFF_2026-05-08.md.

## Risks To Watch

- Do not start gameplay callback wiring while S2 hardware force is unresolved.
- Keep test-mode changes isolated and reversible so production path is not polluted.
- Preserve historical handoff filenames when reorganizing docs to avoid losing timeline continuity.

## Session End State

- Handoff written and pushed target: this file
- Next operator action: begin at Morning Start Plan step 1
