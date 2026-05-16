# INA Bench Truth Table (Rev B)

## Purpose

Capture measured channel-to-rail behavior so INA mapping is trusted before layout freeze.

## Test Conditions

| Field | Value |
|---|---|
| Date | TBD |
| Board revision under test | TBD |
| Supply configuration | TBD |
| Load used | TBD |
| Firmware commit/tag | TBD |

## Measurement Table

| INA Device Addr | INA Channel | Expected Rail | Measured Rail | Expected Current | Measured Current | Pass/Fail | Notes |
|---|---|---|---|---|---|---|---|
| TBD | CH1 | TBD | TBD | TBD | TBD | TBD | |
| TBD | CH2 | TBD | TBD | TBD | TBD | TBD | |
| TBD | CH3 | TBD | TBD | TBD | TBD | TBD | |
| TBD | CH4 | TBD | TBD | TBD | TBD | TBD | |
| TBD | CH5 | TBD | TBD | TBD | TBD | TBD | |

## Exit Criteria

1. Every INA channel is mapped to exactly one rail.
2. Measured values are consistent with shunt assumptions.
3. Any mismatch is logged as an issue before layout freeze.

## Related Docs

- REGULATOR_HAT_REV_WORK_TRACKER.md
- HAT_SCHEMATIC_ISSUES.md
- REGULATOR_SCHEMATIC_ISSUES.md
