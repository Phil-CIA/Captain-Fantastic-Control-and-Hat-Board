# Regulator + HAT Rev Work Tracker

## Purpose

This is the live working document for the regulator PCB and HAT PCB revision effort.
Use [REGULATOR_HAT_REV_KICKOFF.md](REGULATOR_HAT_REV_KICKOFF.md) as the stable entry point.

## Ownership Model

Single-owner workflow: all implementation and sign-off actions are owned by you.

## Current Phase

Interface freeze and issue triage (off-bench prep mode).

## Top Electrical Risk

Buck converter feedback reference is unstable during power-up.
Before the MPU enables the range-switch MOSFETs, the feedback node is effectively floating.
Observed behavior: converter output can rise toward Vin minus about 1.23V.
Revision intent: make feedback valid at startup so output is controlled from first power-on.

## Live Status

| Item | Status | Owner | Notes |
|---|---|---|---|
| Buck feedback reference during power-up | Open | You | Highest-priority fix. Prevent floating feedback at startup. |
| Regulator to HAT connector pinout | Frozen (Electrical) | You | NC/reserved disposition locked; mechanical/mating confirmation still pending. |
| HAT measurement and control interface | In Progress | You | Interface freeze draft table captured; complete firmware semantic sign-off. |
| Rail-enable GPIO assignments | In Progress | You | SR_Data/GPIO3, SR_CLK/GPIO4, SR_Latch/GPIO1 mapped through 74HC595 path. |
| Rail-enable polarity and boot defaults | Pending Bench | You | Deferred until scope access is available. |
| Mechanical stack-up / connector fit | In Progress | You | Stage A baseline evidence captured; CAD artifacts and MPN pair still pending before Stage B. |
| INA channel-to-rail mapping | Open | You | Bench truth table still needed. |
| Adjustable rail feedback path | Open | You | Confirm independent adjustment behavior. |
| LM2596 feedback-sense topology | In Progress | You | Included in the short regulator close list. |
| USB backfeed protection | In Progress | You | Included in the short regulator close list. |
| ON/OFF gate-drive behavior | In Progress | You | Included in the short regulator close list. |
| Analog current limiting per channel | Open | You | New short close-list item; define one limit path per channel. |

## Workstream Notes

### 1. Interface Freeze

- Lock the connector pinout.
- Lock the measurement/control contract.
- Lock rail-enable polarity and startup defaults.
- Lock mechanical stack height and orientation.

### 2. Regulator PCB Revision

- First, fix startup feedback validity so output is controlled during power-up.
- Move the feedback-sense point to the correct side of the MOSFETs.
- Confirm USB backfeed protection strategy.
- Verify ON/OFF gate-drive behavior.
- Clean up routing and test access.

### 3. HAT PCB Revision

- Confirm INA rail mapping and shunt values.
- Confirm stack fit and mating alignment.
- Confirm any telemetry/control pins do not conflict with firmware assumptions.

### 4. Firmware Alignment

- Replace placeholder enable pins.
- Update pin polarity assumptions.
- Keep telemetry mapping aligned with the schematic.

## Decisions Log

| Date | Decision | Status |
|---|---|---|
| 2026-05-14 | Use a separate kickoff doc plus live work tracker | Confirmed |
| 2026-05-14 | Treat regulator and HAT as one coordinated revision cycle | Confirmed |
| 2026-05-14 | Freeze interface before layout edits | Confirmed |

## Next Actions

1. Complete Stage A docs-only capture protocol in [MECHANICAL_STACK_VERIFICATION.md](MECHANICAL_STACK_VERIFICATION.md) using CAD-workstation screenshots and connector datasheet links, then schedule Stage B physical overlay.
2. Close the short HAT interface notes in [HAT_SCHEMATIC_ISSUES.md](HAT_SCHEMATIC_ISSUES.md) and sync any wording changes into [CONSOLIDATED_REV_B_ISSUES.md](CONSOLIDATED_REV_B_ISSUES.md).
3. Close the short Regulator close list in [REGULATOR_SCHEMATIC_ISSUES.md](REGULATOR_SCHEMATIC_ISSUES.md) and mirror the decisions into [REGULATOR_HAT_REV_DECISIONS_LOG.md](REGULATOR_HAT_REV_DECISIONS_LOG.md).
4. Add analog current limiting per channel to the regulator close list and note the preferred approach in [REGULATOR_SCHEMATIC_ISSUES.md](REGULATOR_SCHEMATIC_ISSUES.md).
5. Record bench measurements in [INA_BENCH_TRUTH_TABLE.md](INA_BENCH_TRUTH_TABLE.md) when bench access resumes.
6. Finalize polarity and boot-default sign-off in [GPIO_PINOUT.md](GPIO_PINOUT.md) when bench access resumes.

## References

- [REGULATOR_HAT_REV_KICKOFF.md](REGULATOR_HAT_REV_KICKOFF.md)
- [PROJECT_SPLIT_ISSUES_AND_PRIORITIES.md](PROJECT_SPLIT_ISSUES_AND_PRIORITIES.md)
- [REGULATOR_BOARD_CHANGE_TRACKER.md](REGULATOR_BOARD_CHANGE_TRACKER.md)
- [STACKED_BOARD_FIRST_POWER_BASELINE.md](STACKED_BOARD_FIRST_POWER_BASELINE.md)
- [GPIO_PINOUT.md](GPIO_PINOUT.md)
- [CONNECTOR_PINOUT_MAP.md](CONNECTOR_PINOUT_MAP.md)
- [CONSOLIDATED_REV_B_ISSUES.md](CONSOLIDATED_REV_B_ISSUES.md)
- [REGULATOR_SCHEMATIC_ISSUES.md](REGULATOR_SCHEMATIC_ISSUES.md)
- [HAT_SCHEMATIC_ISSUES.md](HAT_SCHEMATIC_ISSUES.md)
- [INA_BENCH_TRUTH_TABLE.md](INA_BENCH_TRUTH_TABLE.md)
- [MECHANICAL_STACK_VERIFICATION.md](MECHANICAL_STACK_VERIFICATION.md)
- [REGULATOR_HAT_REV_DECISIONS_LOG.md](REGULATOR_HAT_REV_DECISIONS_LOG.md)