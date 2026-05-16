# Today Plan - 2026-05-16

Purpose: Execute a clean, low-risk path from repo cleanup and bench validation to initial game audio integration.

## Source of truth used for this plan
- `MACHINE_TRANSFER_HANDOFF_2026-04-30.md` (latest in working tree)
- `FIRMWARE_INTEGRATION_HANDOFF_2026-04-29.md`
- Commit `f7ed650` history docs:
  - `MORNING_START_HANDOFF_2026-05-08.md` (includes switch recovery addendum)
  - `RECOVERY_TO_GAMEPLAY_PLAN_2026-05-12.md`

Note: commit `7e1a312` is not present in this repository refs after fetch. If that hash is from another repo, use that repo as the source for any additional handoff notes.

## Hard gates for today
1. Do not start gameplay/audio coupling until switch recovery gate passes.
2. Keep S2 protections active unless relay workaround is fully installed and verified.
3. If matrix link health fails for ~1000 ms, stop and fix transport first.

## Execution order

### Phase 1 - Repo and workspace cleanup (matrix + control)
Goal: clean handoff history and reduce confusion before bench work.

1. Pull latest in all repos:
   - Captain-Fantastic-Control-and-Hat-Board
   - Pinball-Sw-Lamp-Matrix-
   - Captain-Fantastic-home-edition
2. In each repo, create/verify a handoff archive folder.
3. Move dated handoff/transfer docs into archive (no renames).
4. Add/update one chronological index page listing all handoffs.
5. Confirm top-level status doc matches actual firmware/hardware state.

Exit criteria:
- No ambiguous "latest handoff" state.
- Session docs are organized and discoverable.

### Phase 2 - Matrix + control bench verification
Goal: prove stable control/matrix baseline before feature work.

1. Flash/verify known test baseline on control board.
2. Verify matrix transport health:
   - packet and read/write counters increase cleanly
   - no sustained checksum/reject noise
3. Run switch quality gate captures:
   - 30s quiet baseline
   - 15-20 deliberate presses on at least 3 target switches
   - verify one dominant bit per intentional press window
4. Validate direct inputs:
   - Tilt: 20 deliberate presses, no misses/false positives
   - Start: 20 deliberate presses, no misses/false positives
5. Verify lamp delivery path:
   - 10 intentional updates, readback matches commanded state

Exit criteria:
- Switch gate passed.
- Direct-input debounce clean.
- Lamp write/readback reliable.

### Phase 3 - S2 status check (parallel stream)
Goal: close S2 risk before broader gameplay behavior.

1. If relay workaround is still pending, enforce manual S2 limits:
   - max 2-3 activations per group
   - 10-15s between attempts
   - 60s between groups
2. If relay is installed, verify full-force outhole behavior and retire temporary limit policy.

Exit criteria:
- S2 is either protected by policy or verified on relay path.

### Phase 4 - Integrate game audio and sound routines
Start only if Phases 2 and 3 pass.

1. Wire switch/solenoid events to existing audio queue hooks.
2. Map first-pass sounds:
   - bumper hit -> bumper sound
   - sling hit -> sling sound
   - outhole/drain -> drain sound
   - start/game transition -> mode transition sound
3. Validate timing:
   - no audible lag on event playback
   - no event spam from bounce/noise
4. Validate interference:
   - no added noise or resets during solenoid events
5. Capture event-to-sound mapping and commit notes.

Exit criteria:
- At least one end-to-end flow confirmed:
  switch -> solenoid action -> lamp update -> sound effect.

## End-of-day deliverables
1. Updated handoff doc with pass/fail gates and blockers.
2. Commit(s) for:
   - docs cleanup/index updates
   - any gate or config changes
   - audio wiring changes (if started)
3. Short next-session start checklist (first 5 commands + first 3 validation checks).

## If blocked
- If switch gate fails: stop gameplay/audio work and continue only with capture + isolation.
- If S2 force remains unresolved: keep S2 out of normal gameplay path and proceed with non-S2 audio/event wiring only.
