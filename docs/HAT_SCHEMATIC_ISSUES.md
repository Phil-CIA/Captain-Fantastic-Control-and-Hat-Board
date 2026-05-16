# HAT Schematic Issues (Rev B)

## Purpose

Track HAT-board electrical and interface issues that must be resolved or dispositioned before fabrication.

## Issue Table

| Local ID | Severity | Status | Type | Summary | Verification Gate |
|---|---|---|---|---|---|
| REVB-HAT-001 | Critical | In Progress | Mechanical | Connector alignment and mating are not yet confirmed for the next revision. | Must-fix before layout freeze |
| REVB-HAT-002 | High | In Progress | Interface | HAT measurement and control interface contract draft is captured; pending sign-off. | Freeze before layout |
| REVB-HAT-003 | High | Open | Mapping | INA channel-to-rail mapping and shunt assumptions need bench truth-table validation. | Verify before layout freeze |
| REVB-HAT-004 | Medium | Open | Mechanical | Stack-up, connector orientation, and keepouts need 1:1 overlay confirmation. | Verify before fab |
| REVB-HAT-005 | Medium | In Progress | Firmware-Coupled | Telemetry/control pins are mapped; firmware semantic sign-off still pending. | Freeze before layout |

## Simple Sign-Off Notes

- Feedback signals: keep them as `telemetry.*` fields and assign the ADC channels later.
- Control signals: keep them as `control.*` fields and leave the exact 0/1 meaning open until polarity is verified.
- Power/path signals: keep `+5V_Control`, `Incoming(+)`, `Incoming(-)`, and `+V_Adj_Channel` as simple named rails, not GPIO commands.
- Bench-dependent fields stay open in [GPIO_PINOUT.md](GPIO_PINOUT.md) and [INA_BENCH_TRUTH_TABLE.md](INA_BENCH_TRUTH_TABLE.md).

## Related Docs

- CONSOLIDATED_REV_B_ISSUES.md
- REGULATOR_HAT_REV_WORK_TRACKER.md
- REV_B_ACTION_LIST.md
