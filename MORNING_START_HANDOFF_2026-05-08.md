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

## Switch Recovery Addendum (2026-05-11)

### What We Know

1. Known-good matrix checkpoint exists from the working perspective: `aca9ac0` (matrix chunked OLED milestone, 2026-05-05).
2. Latest handoff (2026-05-10) indicates transport health is stable:
   - `rx_pkts` increasing on matrix
   - `wr_ok` and `rd_ok` increasing on control
   - checksum/reject alarms no longer noisy
3. Current failure mode is switch isolation/mapping quality, not basic link up/down.
4. Current control firmware has a strict burst-drop guard (`MATRIX_MAX_RISING_EDGES_PER_POLL = 4`) that can suppress entire poll windows.
5. Mapping-mode concept (lamps forced off during mapping capture) is a valid mitigation and must be verified in the exact flashed binary before further tuning.

### What We Need To Know

1. Are real switch presses being dropped by filtering thresholds (control side) versus lost on matrix scan/debounce (matrix side)?
2. Is lamp-off mapping mode active in the actual build being tested?
3. Did row/column mapping or active-row assumptions drift between `aca9ac0` and current cleanup state?
4. Do dropped-burst counts correlate directly with deliberate press windows?
5. Is the current debounce cadence suppressing short but valid press events?

### Recovery-First Plan (No Gameplay Features Until This Passes)

1. Freeze A/B references:
   - A = known-good behavior target (`aca9ac0`)
   - B = current behavior target (latest split-protocol head)
2. Run identical captures for A and B on same hardware and wiring:
   - 30s quiet baseline
   - 15-20 deliberate presses on one target switch
   - repeat on two additional switches
3. Compare outcomes:
   - If A clean and B fails, software regression confirmed.
   - If both fail, prioritize hardware/wiring/mechanics isolation.
4. If software regression is confirmed, isolate in this order:
   - force lamp-off mapping mode
   - temporarily relax burst-drop threshold for capture build
   - reduce debounce ticks for capture build
   - verify row/col mapping and active-row limits
5. Lock acceptance gate before gameplay coupling resumes:
   - each tested switch yields one dominant bit in press windows
   - intentional presses are not suppressed by burst filter

### Hard Stop Rule

Do not proceed to scoring, gameplay callback wiring, or mode expansion until the switch recovery acceptance gate is met and repeatable.

---

## Integration Reference: Recovery-to-Gameplay Plan (May 12–13)

A comprehensive next-phase execution plan has been created to advance from baseline validation (timing locked, I2C healthy, S3–S6 solenoid working) toward playable behavior.

**See**: [RECOVERY_TO_GAMEPLAY_PLAN_2026-05-12.md](RECOVERY_TO_GAMEPLAY_PLAN_2026-05-12.md)

### Key Highlights from That Plan

1. **What We Know** (May 12 End-of-Day Status)
   - Matrix timing locked and acceptance test passed
   - I2C transport stable under continuous polling
   - Solenoid S3–S6 verified working on TPS
   - S2 relay workaround pending installation (parallel stream, not blocking)

2. **What We Need To Know** (Blocking Criteria)
   - Switch detection quality: one dominant bit per press (A/B capture validation required)
   - Tilt & Start direct-input paths clean (no cross-talk with matrix)
   - Lamp delivery to headbox working without frame corruption

3. **S2 Coil Protection Policy** (During Relay Installation & Limited Testing)
   - Max 2–3 activations per group
   - 10–15s cool-down between attempts
   - 60s cool-down between groups
   - Operator observes coil temperature/sound for abort signals
   - Policy suspended after relay installation complete

4. **Parallel Work Streams** (May 13–14, ~6–8 hours total)
   - **Stream A (Hardware)**: Install S2 relay, verify 24V force recovery (~2–4 hours)
   - **Stream B (Software)**: Execute switch A/B capture validation, run gate analysis, apply feature gates if needed (~4–6 hours)

5. **Gameplay Integration** (Begins After Switch Gate Passes)
   - Mode scaffolding (Boot → Test → Attract → Gameplay → Error)
   - Solenoid coupling (switch edge → solenoid action)
   - Score & lamp state wiring

6. **Definition of Done**
   - Switch validation gate passed ✅
   - S2 coil protection enforced or relay installed ✅
   - One end-to-end switch→solenoid→lamp flow working ✅
   - All docs updated and pushed ✅

**Execution Sequence**: Start with parallel streams (relay install + switch captures). Switch gate result drives gameplay integration timeline. Reference that plan for detailed command sequences, feature gates, and risk mitigations.
