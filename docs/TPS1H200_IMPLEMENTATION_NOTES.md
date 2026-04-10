# TPS1H200A-Q1 Implementation Notes

This note turns the selected solenoid-driver part into a concrete **starting-point hookup plan** for the next control-board respin.

## Confirmed device facts
From the TI product information / datasheet:
- **`TPS1H200A-Q1`**
- **single-channel smart high-side switch**
- **3.4 V to 40 V** operating range
- **200 mΩ** typical on-resistance
- **adjustable current limit** with an external resistor
- configurable post-fault behavior through the **`DELAY`** pin:
  - holding mode
  - latch-off mode with adjustable delay
  - auto-retry mode
- built-in protection / diagnostics for overload, short-to-ground, thermal fault, and inductive-load switching

## Per-channel wiring intent
For each solenoid output (`S2`..`S6`):

1. `SOL_24V` feeds the device supply side
2. the switched output goes to `SOL_OUT_Sx`, then to the connector / coil
3. `SOL_EN_Sx` from the ESP32 drives the logic-enable input
4. the device ground returns directly to `SOL_GND`
5. place **local supply decoupling close to the device**
6. route the fault / diagnostic signal to a **test pad** and optionally to an MCU input such as `SOL_FAULT_Sx`
7. keep the **current-limit programming** and **`DELAY` behavior selection** easy to tune during bench bring-up

## Initial Rev B defaults
For the first schematic / layout pass:
- use **one device per output** for `S2`, `S3`, `S4`, `S5`, and `S6`
- keep the current firmware naming aligned with the new net names where possible
- avoid rebuilding the old discrete PMOS gate-drive stage around the new part
- keep high-current traces short and reasonably wide
- give each device some copper area for thermal spreading
- keep the 24 V feed and output path physically direct

## Bring-up recommendation
Until the actual coil envelope is confirmed, the board should stay easy to tune:
- keep the current-limit setpoint adjustable by resistor choice
- keep the `DELAY` / fault-response choice easy to change during bench validation
- prefer a protected, diagnosable configuration over a “blind” raw MOSFET-style drive
- use `docs/TPS1H200_CL_DELAY_BENCH_TABLE.md` to record CL and DELAY trials per channel

## Still to confirm before freezing values
- target coil DC resistance
- expected pulse width
- repeat rate / duty cycle
- whether any output needs true hold-current behavior, or only short pulse actuation

## Practical meaning
The architecture choice is now made.

The remaining hardware work is to convert the old discrete solenoid block in the KiCad schematic into this repeated `TPS1H200A-Q1` pattern and then tune the support values from the real coil measurements.