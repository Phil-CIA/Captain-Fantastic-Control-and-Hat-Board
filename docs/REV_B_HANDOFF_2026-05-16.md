# Rev B Handoff (2026-05-16)

Purpose: close out the current documentation-and-triage pass for the regulator + HAT Rev B effort and give the next machine a clean starting point for actual issue rectification.

## Current State

This effort is at the transition between setup and rectification.

The documentation scaffold is now in place and internally aligned:
- kickoff doc exists
- live tracker exists
- decisions log exists
- connector electrical pinout is frozen
- GPIO shift-register mapping is captured
- HAT and regulator issue lists are simplified into short close lists
- mechanical verification has a docs-only Stage A path
- bench-gated items are explicitly deferred

The remaining work is no longer process setup.
The remaining work is closing real design decisions and verifying them.

## What Was Completed In This Effort

1. Established the Rev B docs structure around kickoff, live tracker, issue inventory, decisions log, interface docs, and mechanical verification.
2. Froze the regulator-to-HAT connector electrical pin map, including NC_Reserved disposition.
3. Captured shared shift-register GPIO mapping:
   - GPIO3 = SR_Data
   - GPIO4 = SR_CLK
   - GPIO1 = SR_Latch
4. Simplified the HAT and regulator worksheets into short close lists suitable for single-owner use.
5. Added analog current limiting per channel to the regulator close list, tracker, issue inventory, and decisions log.
6. Split off bench-dependent work so off-bench progress can continue cleanly.

## Key Docs To Start From

Start here:
- REGULATOR_HAT_REV_KICKOFF.md
- REGULATOR_HAT_REV_WORK_TRACKER.md
- REGULATOR_HAT_REV_DECISIONS_LOG.md

Supporting docs:
- CONNECTOR_PINOUT_MAP.md
- GPIO_PINOUT.md
- HAT_SCHEMATIC_ISSUES.md
- REGULATOR_SCHEMATIC_ISSUES.md
- CONSOLIDATED_REV_B_ISSUES.md
- MECHANICAL_STACK_VERIFICATION.md

Bench-deferred docs:
- GPIO_PINOUT.md
- INA_BENCH_TRUTH_TABLE.md

## Confirmed Decisions

- Use separate kickoff doc plus live tracker.
- Treat regulator and HAT as one coordinated Rev B cycle.
- Freeze interface before layout edits.
- Use firmware naming:
  - control.* for commands
  - telemetry.* for measurements
  - _mv for millivolts
  - _present for boolean presence flags
- Shift-register GPIO control mapping:
  - SR_Data = GPIO3
  - SR_CLK = GPIO4
  - SR_Latch = GPIO1
- Connector electrical pin map is frozen, with unused pins marked NC_Reserved.

## Open Decisions

1. Final startup feedback-fix path.
2. Final USB backfeed protection path.
3. Final connector mating MPN pair and stack-height sign-off.
4. Final rail-enable polarity and boot defaults.
5. Final HAT semantic contract.
6. Final regulator electrical contract.
7. Final analog current limiting per channel approach.

## Most Important Unresolved Technical Issue

Buck converter feedback reference is unstable during power-up.

Before the MPU enables the range-switch MOSFETs, the feedback node can float.
That allows converter output to rise toward Vin minus about 1.23 V.
The next real electrical design pass should fix startup feedback validity first.

## What Is Ready For Immediate Work

Off-bench:
- finish Stage A mechanical capture in MECHANICAL_STACK_VERIFICATION.md
- close HAT semantic wording in HAT_SCHEMATIC_ISSUES.md
- close regulator close-list decisions in REGULATOR_SCHEMATIC_ISSUES.md
- choose a simple analog current limiting strategy per channel

Bench-required:
- rail-enable polarity sign-off
- boot-default sign-off
- INA channel-to-rail truth table
- physical connector mating and stack verification

## Recommended Next Effort

Treat the next effort as problem rectification pass 1.

Priority order:
1. Mechanical Stage A closure
2. Regulator startup feedback fix path
3. Simple final regulator close-list decisions
4. HAT semantic contract closure
5. Bench-only verification later

## Constraints

- Current active workspace is documentation-focused.
- Bench work is not available right now.
- Some mechanical closure still depends on CAD workstation artifacts and connector datasheets.
- Keep docs simple; avoid re-expanding into large worksheet formats.

## Resume Instruction For Next Agent

Resume from REGULATOR_HAT_REV_WORK_TRACKER.md.
Do not rebuild process docs.
Assume the documentation scaffold is complete enough.
Focus on closing remaining design decisions and converting open tracker items into explicit chosen solutions.