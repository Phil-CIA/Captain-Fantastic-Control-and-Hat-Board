# Regulator + HAT Rev Decisions Log

## Purpose

Record sign-off decisions for Rev B so unresolved assumptions do not leak into fabrication.

## Decision Log

| Date | Area | Decision | Rationale | Impacted Docs | Status |
|---|---|---|---|---|---|
| 2026-05-16 | Process | Use separate kickoff doc plus live tracker. | Stable entry point plus active execution log. | REGULATOR_HAT_REV_KICKOFF.md, REGULATOR_HAT_REV_WORK_TRACKER.md | Confirmed |
| 2026-05-16 | Scope | Treat regulator and HAT as one coordinated Rev B cycle. | Shared interface and coupled risks. | CONSOLIDATED_REV_B_ISSUES.md | Confirmed |
| 2026-05-16 | Process | Freeze interface before layout edits. | Prevent routing churn and rework. | CONNECTOR_PINOUT_MAP.md, GPIO_PINOUT.md | Confirmed |
| 2026-05-16 | Interface | Capture SR control GPIO mapping as GPIO3=SR_Data, GPIO4=SR_CLK, GPIO1=SR_Latch. | Removes placeholder pin ambiguity and unblocks pinout freeze review. | GPIO_PINOUT.md, CONSOLIDATED_REV_B_ISSUES.md | Confirmed |
| 2026-05-16 | Interface | Freeze connector electrical pin map with NC_Reserved disposition on unused pins (J1: 7,8,12,13,14,15; J2: 10,11,12,13,15). | Locks non-bench pin-function scope and prevents accidental routing onto reserved pins. | CONNECTOR_PINOUT_MAP.md, REGULATOR_HAT_REV_WORK_TRACKER.md, CONSOLIDATED_REV_B_ISSUES.md | Confirmed |
| 2026-05-16 | Interface | Adopt proposed firmware naming convention: `control.*` for commands, `telemetry.*` for measurements, `_mv` for millivolts, `_present` for boolean presence flags. | Gives the HAT interface a consistent namespace while keeping off-bench sign-off readable. | HAT_SCHEMATIC_ISSUES.md, REGULATOR_HAT_REV_WORK_TRACKER.md, CONSOLIDATED_REV_B_ISSUES.md | Confirmed |

## Open Decision Slots

| Date | Area | Decision Needed | Owner | Due |
|---|---|---|---|---|
| TBD | Electrical | Final startup feedback-fix implementation path | You | TBD |
| TBD | Protection | Final USB backfeed solution for production board | You | TBD |
| TBD | Mechanical | Final connector mating and stack-height sign-off | You | TBD |
| TBD | Interface | Rail-enable polarity and boot-default freeze sign-off for ISET_MPU_5V, ISET_MPU_3V3, ISET_MPU_Channel_3, and 3.3_Select | You | TBD |
| TBD | Interface | Final HAT measurement/control semantic contract sign-off (field names, allowed states, defaults, and error handling) | You | TBD |
| TBD | Electrical | Final regulator electrical contract sign-off (startup feedback fix, USB backfeed protection, LM2596 sense topology, ON/OFF behavior, adjustable feedback path, INA mapping, and test-point access) | You | TBD |
| TBD | Electrical | Analog current limiting per channel strategy | You | TBD |

## Related Docs

- CONSOLIDATED_REV_B_ISSUES.md
- REGULATOR_HAT_REV_WORK_TRACKER.md
- REGULATOR_HAT_REV_KICKOFF.md
