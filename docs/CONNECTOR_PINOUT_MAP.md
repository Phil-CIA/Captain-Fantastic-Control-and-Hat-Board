# Regulator <-> HAT Connector Pinout Map (Rev B)

## Purpose

Freeze board-to-board connector pin intent before layout work.
This is the source table for power, control, and telemetry signals across the two boards.

## Connector Definition

| Field | Value |
|---|---|
| Regulator board connector ref | J1, J2 |
| HAT board connector ref | Mating connectors for J1, J2 (exact refs TBD) |
| Mating part numbers | TBD |
| Pin-1 orientation marking | TBD |
| Stack-height target | TBD |

## Pin Map

| Connector | Pin | Signal Name | Direction | Voltage Domain | Function | Notes | Status |
|---|---|---|---|---|---|---|---|
| J1 | 1 | GND | Shared return | 0V | Ground | Common ground | Frozen (Electrical) |
| J1 | 2 | GND | Shared return | 0V | Ground | Common ground | Frozen (Electrical) |
| J1 | 3 | GND | Shared return | 0V | Ground | Common ground | Frozen (Electrical) |
| J1 | 4 | Feedback_5V | Regulator -> Control | Analog | 5V feedback sense | Verify ADC destination | Frozen (Electrical) |
| J1 | 5 | ISET_MPU_5V | Control -> Regulator | 3V3 logic | 5V set/control | Via shift register output | Frozen (Electrical) |
| J1 | 6 | +5V_reg | Regulator -> HAT/Load | 5V power | Regulated 5V output | Confirm load path in layout | Frozen (Electrical) |
| J1 | 7 | NC_Reserved | None | N/A | Reserved | No-connect by rev freeze (do not route/use). | Frozen (NC) |
| J1 | 8 | NC_Reserved | None | N/A | Reserved | No-connect by rev freeze (do not route/use). | Frozen (NC) |
| J1 | 9 | Feedback_3.3 | Regulator -> Control | Analog | 3.3V feedback sense | Verify ADC destination | Frozen (Electrical) |
| J1 | 10 | ISET_MPU_3V3 | Control -> Regulator | 3V3 logic | 3.3V set/control | Via shift register output | Frozen (Electrical) |
| J1 | 11 | +3.3V_reg | Regulator -> HAT/Load | 3V3 power | Regulated 3.3V output | Confirm load grouping | Frozen (Electrical) |
| J1 | 12 | NC_Reserved | None | N/A | Reserved | No-connect by rev freeze (do not route/use). | Frozen (NC) |
| J1 | 13 | NC_Reserved | None | N/A | Reserved | No-connect by rev freeze (do not route/use). | Frozen (NC) |
| J1 | 14 | NC_Reserved | None | N/A | Reserved | Marked no-connect in screenshot; retained as no-connect. | Frozen (NC) |
| J1 | 15 | NC_Reserved | None | N/A | Reserved | Marked no-connect in screenshot; retained as no-connect. | Frozen (NC) |
| J2 | 1 | GND | Shared return | 0V | Ground | Common ground | Frozen (Electrical) |
| J2 | 2 | GND | Shared return | 0V | Ground | Common ground | Frozen (Electrical) |
| J2 | 3 | GND | Shared return | 0V | Ground | Common ground | Frozen (Electrical) |
| J2 | 4 | Feedback_Adj_Channel | Regulator -> Control | Analog | Adjustable channel feedback | Verify ADC destination | Frozen (Electrical) |
| J2 | 5 | ISET_MPU_Channel_3 | Control -> Regulator | 3V3 logic | Adjustable channel set/control | Via shift register output | Frozen (Electrical) |
| J2 | 6 | 3.3_Select | Control -> Regulator | 3V3 logic | Rail/select control | Via shift register output | Frozen (Electrical) |
| J2 | 7 | +5V_Control | 5V rail presence | 5V | Power/control rail net | Treated as rail net, not direct GPIO | Frozen (Electrical) |
| J2 | 8 | Incoming(-) | Input return | Input domain | External/input return | Pair with Incoming(+) | Frozen (Electrical) |
| J2 | 9 | Incoming(+) | Input supply | Input domain | External/input positive | Pair with Incoming(-) | Frozen (Electrical) |
| J2 | 10 | NC_Reserved | None | N/A | Reserved | Marked no-connect in screenshot; retained as no-connect. | Frozen (NC) |
| J2 | 11 | NC_Reserved | None | N/A | Reserved | Marked no-connect in screenshot; retained as no-connect. | Frozen (NC) |
| J2 | 12 | NC_Reserved | None | N/A | Reserved | Marked no-connect in screenshot; retained as no-connect. | Frozen (NC) |
| J2 | 13 | NC_Reserved | None | N/A | Reserved | Marked no-connect in screenshot; retained as no-connect. | Frozen (NC) |
| J2 | 14 | +V_Adj_Channel | Regulator -> HAT/Load | Adjustable rail | Adjustable output rail | Confirm nominal range | Frozen (Electrical) |
| J2 | 15 | NC_Reserved | None | N/A | Reserved | No-connect by rev freeze (do not route/use). | Frozen (NC) |

## Freeze State

Electrical pin-function freeze: COMPLETE.
Mechanical and mating freeze: PENDING (orientation, stack-up, and part-number confirmation).

## NC Disposition (This Rev)

- J1 pins 7, 8, 12, 13, 14, 15: NC_Reserved, do not route or assign functionality.
- J2 pins 10, 11, 12, 13, 15: NC_Reserved, do not route or assign functionality.

## Freeze Criteria

1. Every connector pin has one approved function.
2. Direction and voltage domain are defined for each signal.
3. Pin-1 orientation and mating references are verified in schematic and PCB.
4. Mechanical overlay check completed and logged.

## Related Docs

- REGULATOR_HAT_REV_WORK_TRACKER.md
- MECHANICAL_STACK_VERIFICATION.md
- REGULATOR_HAT_REV_DECISIONS_LOG.md
