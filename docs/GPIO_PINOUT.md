# GPIO Pinout (Rev B Freeze)

## Purpose

Freeze the real rail-enable and related GPIO assignments for the regulator + HAT revision.
This replaces placeholder pin choices before schematic and layout freeze.

## Status

| Item | Status | Notes |
|---|---|---|
| Rail-enable GPIO assignments | In Progress | Legacy control net names restored and SR control GPIOs identified. |
| Rail-enable polarity | Pending Bench | Requires scope verification at 74HC595 outputs. |
| Boot default states | Pending Bench | Requires power-cycle capture before and after first SR latch. |
| Firmware alignment check | In Progress | SR_Data/GPIO3, SR_CLK/GPIO4, SR_Latch/GPIO1 confirmed from rev screenshots. |

## Control Path Summary

- Regulator control outputs are driven through 74HC595 shift registers.
- Shared shift-register control nets:
	- SR_Data from GPIO3
	- SR_CLK from GPIO4
	- SR_Latch from GPIO1

## Pin Assignment Table

| Signal | Direction | MCU Pin | Active Level | Boot Default | Connected Net | Status |
|---|---|---|---|---|---|---|
| ISET_MPU_5V | MCU -> U4 (74HC595) -> Regulator | GPIO3 (SR_Data), GPIO4 (SR_CLK), GPIO1 (SR_Latch) | TBD | Undefined until SR init | U4.Q0 -> ISET_MPU_5V | Mapped |
| ISET_MPU_3V3 | MCU -> U4 (74HC595) -> Regulator | GPIO3 (SR_Data), GPIO4 (SR_CLK), GPIO1 (SR_Latch) | TBD | Undefined until SR init | U4.Q7 -> ISET_MPU_3V3 | Mapped |
| ISET_MPU_Channel_3 | MCU -> U5 (74HC595) -> Regulator | GPIO3 (SR_Data), GPIO4 (SR_CLK), GPIO1 (SR_Latch) | TBD | Undefined until SR init | U5.Q1 -> ISET_MPU_Channel_3 | Mapped |
| 3.3_Select | MCU -> U5 (74HC595) -> Regulator | GPIO3 (SR_Data), GPIO4 (SR_CLK), GPIO1 (SR_Latch) | TBD | Undefined until SR init | U5.Q0 -> 3.3_Select | Mapped |
| +5V_Control | 5V rail presence -> Regulator/HAT | N/A | N/A | Follows 5V rail presence | +5V_Control | Mapped (not GPIO) |

## Polarity And Boot-Default Sign-Off Matrix

| Signal | Provisional Boot State | Verify Method | Pass Condition | Status |
|---|---|---|---|---|
| ISET_MPU_5V | Not frozen | Scope U4.Q0 during power-up and first SR latch | Output state and resulting rail behavior match firmware intent | Open |
| ISET_MPU_3V3 | Not frozen | Scope U4.Q7 during power-up and first SR latch | Output state and resulting rail behavior match firmware intent | Open |
| ISET_MPU_Channel_3 | Not frozen | Scope U5.Q1 during power-up and first SR latch | Output state and resulting rail behavior match firmware intent | Open |
| 3.3_Select | Not frozen | Scope U5.Q0 during power-up and first SR latch | Output state and selected mode match firmware intent | Open |

## Bench Capture Log

| Date | Firmware Build | Signal | Measured Boot Level | Measured Post-Latch Level | Interpreted Polarity | Rail/Mode Effect | Result | Notes |
|---|---|---|---|---|---|---|---|---|
| TBD | TBD | ISET_MPU_5V | TBD | TBD | TBD | TBD | Open | |
| TBD | TBD | ISET_MPU_3V3 | TBD | TBD | TBD | TBD | Open | |
| TBD | TBD | ISET_MPU_Channel_3 | TBD | TBD | TBD | TBD | Open | |
| TBD | TBD | 3.3_Select | TBD | TBD | TBD | TBD | Open | |

Bench note:
- Capture one power-cycle scope shot per signal at the shift-register output pin and attach file names in Notes.

Resume note:
- Bench-dependent verification is intentionally deferred until bench access is available.
- Keep Active Level and Boot Default fields as TBD until measurements are logged.

## Startup Sequence Requirement

1. Hold SR_Data, SR_CLK, and SR_Latch in known idle states at reset.
2. Shift a safe default image into both 74HC595 devices.
3. Latch once to force deterministic outputs.
4. Only then enable any rail behavior that depends on ISET or 3.3_Select.

## Freeze Criteria

1. All enable signals have concrete MCU pin assignments.
2. Active polarity is documented and verified in hardware and firmware.
3. Safe boot defaults are defined for power-up.
4. This file is referenced by the live tracker and signed in the decisions log.

## Related Docs

- REGULATOR_HAT_REV_WORK_TRACKER.md
- CONNECTOR_PINOUT_MAP.md
- REGULATOR_HAT_REV_DECISIONS_LOG.md
