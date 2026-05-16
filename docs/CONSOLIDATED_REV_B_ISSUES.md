# Consolidated Rev B Issues

## Purpose

Single issue inventory for regulator + HAT revision planning, triage, and sign-off.
This list merges current local docs findings with imported linked-tracker alias IDs.

## Priority Legend

- Critical: blocks safe bring-up or layout freeze
- High: must be resolved or dispositioned before layout freeze
- Medium: verify before fab or explicitly defer with rationale

## Issue Inventory

| Local ID | External Alias | Board | Priority | Status | Category | Summary | Gate |
|---|---|---|---|---|---|---|---|
| REVB-REG-001 | RB-001 | Regulator | Critical | In Progress | Electrical | Buck feedback reference startup fix is in the short regulator close list. | Must-fix before layout freeze |
| REVB-REG-002 | RB-002 | Regulator | High | In Progress | Protection | USB backfeed production strategy is in the short regulator close list. | Must-fix before layout freeze |
| REVB-REG-003 | RB-003 | Regulator | High | In Progress | Schematic | LM2596 feedback-sense topology is in the short regulator close list. | Must-fix before layout freeze |
| REVB-REG-004 | RB-004 | Regulator | Medium | In Progress | Validation | ON/OFF gate-drive behavior is in the short regulator close list. | Verify before fab |
| REVB-REG-005 | RB-005 | Regulator | Medium | In Progress | Validation | Adjustable rail feedback path behavior is in the short regulator close list. | Verify before layout freeze |
| REVB-REG-006 | RB-006 | Shared | Medium | In Progress | Mapping | INA channel-to-rail mapping is in the short regulator close list. | Verify before layout freeze |
| REVB-REG-007 | RB-007 | Shared | High | In Progress | Interface | Shift-register GPIO assignments are mapped; finalize as frozen after review. | Freeze before layout |
| REVB-REG-008 | RB-008 | Shared | High | In Progress | Interface | Boot defaults and active polarity still need explicit sign-off. | Freeze before layout |
| REVB-REG-009 | RB-009 | Regulator | Medium | In Progress | Layout | Test-point and measurement access is in the short regulator close list. | Verify before fab |
| REVB-REG-010 | RB-010 | Regulator | Medium | Open | Validation | Analog current limiting per channel needs a simple final approach. | Verify before layout freeze |
| REVB-HAT-001 | N/A | HAT | Critical | In Progress | Mechanical | Electrical pin map and NC disposition are frozen; Stage A doc-backed baseline is captured; connector alignment/mating still pending CAD+physical confirmation. | Must-fix before layout freeze |
| REVB-HAT-002 | N/A | HAT | High | In Progress | Interface | Measurement/control interface contract draft and semantic worksheet are prefilled; pending final review/sign-off. | Freeze before layout |
| REVB-HAT-003 | N/A | HAT | High | Open | Mapping | INA channel-to-rail mapping and shunt assumptions need bench truth-table validation. | Verify before layout freeze |
| REVB-HAT-004 | N/A | HAT | Medium | Open | Mechanical | Stack-up, connector orientation, and keepouts need 1:1 overlay confirmation. | Verify before fab |
| REVB-HAT-005 | N/A | HAT | Medium | In Progress | Firmware-Coupled | Telemetry/control pins are mapped in draft; proposed firmware field names are prefilled; semantic sign-off still pending. | Freeze before layout |

## Immediate Execution Order

1. Complete connector mechanical overlay and mating verification.
2. Finalize polarity and boot-default sign-off for frozen GPIO mapping.
3. Lock startup feedback fix path and LM2596 sense strategy.
4. Run INA mapping bench truth-table capture.
5. Update tracker statuses and decisions log.

## Related Docs

- REGULATOR_HAT_REV_KICKOFF.md
- REGULATOR_HAT_REV_WORK_TRACKER.md
- REGULATOR_SCHEMATIC_ISSUES.md
- HAT_SCHEMATIC_ISSUES.md
- GPIO_PINOUT.md
- CONNECTOR_PINOUT_MAP.md
