# Regulator + HAT Rev Kickoff

## Scope

This revision covers the development-station regulator PCB and the HAT PCB as one coordinated hardware cycle.

## Goal

Produce a production-candidate revision with the board interfaces frozen, the known electrical risks fixed, and the bring-up path documented before fabrication.

## Primary Problem Statement

During power-up, the buck converter feedback reference is unstable.
The feedback pin has no valid reference until the MPU enables the range-switch MOSFETs.
In that window, the converter can rise toward Vin minus about 1.23V.
This is the highest-priority electrical fix for the revision.

## Plain-Language Note On Connector Pinout

Connector pinout means a simple table that says what each board-to-board pin is used for.
Example: which pin carries 5V, which pin is ground, which pin is enable, and which pin is measurement.

## Freeze Before Layout

1. Regulator to HAT connector pinout.
2. HAT measurement and control interface.
3. Real rail-enable GPIO assignments, polarity, and boot defaults.
4. Mechanical stack-up, connector orientation, and keepouts.
5. INA channel-to-rail mapping and shunt assumptions.

## Known Blockers

- Buck feedback reference is invalid at power-up until MPU-controlled range-switch MOSFETs turn on.
- INA current mapping is not yet trustworthy.
- Adjustable rail feedback needs verification against the physical board.
- LM2596 feedback sense needs the long-term schematic fix.
- USB backfeed protection and gate-drive behavior still need final redesign decisions.
- HAT stack fit and connector alignment still need final mechanical confirmation.

## Issue Sign-Off Table

| Item | Status | Decision | Notes |
|---|---|---|---|
| Buck feedback reference during power-up | Open | Must-fix before layout freeze | Fix first. Prevent uncontrolled rise while feedback is floating. |
| Regulator to HAT connector pinout | Open | Freeze before layout | Do not route around an unapproved connector map. |
| HAT measurement and control interface | Open | Freeze before layout | Define what is sensed, controlled, and returned to firmware. |
| Rail-enable GPIO assignments | Open | Freeze before layout | Replace placeholder pins with real hardware assignments. |
| Mechanical stack-up and connector orientation | Open | Freeze before layout | Verify fit with print/overlay before fab. |
| INA channel-to-rail mapping | Open | Verify before layout | Confirm channel, address, and shunt values against the bench. |
| Adjustable rail feedback path | Open | Verify before layout | Confirm the rail is independently adjustable and not default-stuck. |
| LM2596 feedback-sense topology | Open | Redesign fix | Move sense point to the correct side of the MOSFETs. |
| USB backfeed protection | Open | Redesign fix | Keep the bench workaround separate from the final PCB. |
| ON/OFF gate-drive behavior | Open | Verify before fab | Scope the transient behavior before committing the circuit. |

## Regulator PCB Workstream

- Fix feedback-sense topology.
- Fix USB backfeed protection.
- Verify ON/OFF gate-drive circuit behavior.
- Clean up the schematic and layout around power routing and test points.
- Run ERC and DRC after the interface is frozen.

## HAT PCB Workstream

- Lock the measurement and control topology.
- Map INA channels to rails and verify shunt values.
- Lock connector fit, orientation, and stack height.
- Verify any control or telemetry pins against the firmware contract.
- Run ERC and DRC after the interface is frozen.

## Firmware Coupling

- Replace placeholder rail-enable GPIOs with real pin assignments.
- Align active-high or active-low semantics with the board design.
- Keep the telemetry mapping in sync with the HAT schematic.
- Update bring-up commands only after the hardware contract is final.

## Verification Gates

1. Interface freeze review complete.
2. Firmware pin map aligned with the hardware pins.
3. Mechanical fit checked with a print or overlay.
4. ERC clean or fully dispositioned.
5. DRC clean or fully dispositioned.
6. Bring-up checklist prepared for first power.

## Immediate Next Actions

1. Turn the issue list into a single sign-off table.
2. Mark which items are hardware fixes, which are firmware-coupled, and which are deferred.
3. Start the schematic freeze review from the regulator-to-HAT connector outward.
4. Then update the PCB layouts and re-run ERC/DRC.
5. Track live progress in [REGULATOR_HAT_REV_WORK_TRACKER.md](REGULATOR_HAT_REV_WORK_TRACKER.md).

## References

- [REGULATOR_HAT_REV_WORK_TRACKER.md](REGULATOR_HAT_REV_WORK_TRACKER.md)
- [PROJECT_SPLIT_ISSUES_AND_PRIORITIES.md](PROJECT_SPLIT_ISSUES_AND_PRIORITIES.md)
- [REGULATOR_BOARD_CHANGE_TRACKER.md](REGULATOR_BOARD_CHANGE_TRACKER.md)
- [STACKED_BOARD_FIRST_POWER_BASELINE.md](STACKED_BOARD_FIRST_POWER_BASELINE.md)
- [GPIO_PINOUT.md](GPIO_PINOUT.md)
- [HANDOFF_2026-05-14.md](../HANDOFF_2026-05-14.md)
