# Regulator Schematic Issues (Rev B)

## Purpose

Track regulator-board electrical issues that must be resolved or dispositioned before fabrication.

## Issue Table

| Local ID | External Alias | Severity | Status | Type | Summary | Verification Gate |
|---|---|---|---|---|---|---|
| REVB-REG-001 | RB-001 | Critical | In Progress | Electrical | Buck feedback reference can float at startup before range-switch MOSFET enable. | Must-fix before layout freeze |
| REVB-REG-002 | RB-002 | High | In Progress | Protection | USB backfeed protection strategy is captured in the regulator electrical worksheet. | Must-fix before layout freeze |
| REVB-REG-003 | RB-003 | High | In Progress | Schematic | LM2596 feedback-sense point is captured in the regulator electrical worksheet. | Must-fix before layout freeze |
| REVB-REG-004 | RB-004 | Medium | In Progress | Validation | ON/OFF gate-drive transient behavior is captured in the regulator electrical worksheet. | Verify before fab |
| REVB-REG-005 | RB-005 | Medium | In Progress | Validation | Adjustable rail feedback path is captured in the regulator electrical worksheet. | Verify before layout freeze |
| REVB-REG-006 | RB-006 | Medium | In Progress | Mapping | INA channel-to-rail mapping is captured in the regulator electrical worksheet. | Verify before layout freeze |
| REVB-REG-007 | RB-007 | Medium | In Progress | Interface | Shift-register GPIO assignments are mapped; awaiting formal freeze sign-off. | Freeze before layout |
| REVB-REG-008 | RB-008 | Medium | In Progress | Interface | Rail-enable polarity and boot-default assumptions need bench sign-off. | Freeze before layout |
| REVB-REG-009 | RB-009 | Medium | In Progress | Documentation | Regulator test points and measurement access are captured in the regulator electrical worksheet. | Verify before fab |

## Simple Regulator Close List

- Buck feedback: keep the startup fix explicit and simple, then close it in the tracker when the wording is locked.
- USB backfeed: pick one final production strategy and keep the bench workaround out of the final design.
- LM2596 sense: capture the final sense-point change in one sentence and keep it aligned with the layout.
- ON/OFF behavior: leave it open until bench validation returns, but keep the expected behavior note short.
- Analog current limiting: add per-channel analog current limiting so each output has a simple local limit path.
- Adjustable rail / INA / test points: keep each as a short open item with one clear decision needed.

## Notes

- External aliases RB-001 through RB-009 are maintained for tracker compatibility with linked-repo issue history.
- Local IDs are the primary identifiers for this workspace.

## Related Docs

- CONSOLIDATED_REV_B_ISSUES.md
- REGULATOR_HAT_REV_WORK_TRACKER.md
- REV_B_ACTION_LIST.md
