# TPS1H200 CL and DELAY Bench Tuning Table

This table is for first hardware validation of the repeated `TPS1H200A-Q1` channels (`S2`..`S6`).

Use it after logic bring-up is stable and before enabling the real 22-26 V solenoid rail.

## How to use this table
- start on current-limited bench power
- validate one output channel at a time
- record measured current, thermal behavior, and fault behavior
- keep the chosen CL and DELAY population easy to change between runs

## Channel test record
Fill one row per tested channel and configuration.

| Date | Channel | Supply used | Coil/load used | CL population | DELAY population | Mode observed | Peak current | Pulse width | Fault seen | Thermal notes | Result |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| YYYY-MM-DD | S2 | 5 V only / 24 V | Test load / real coil | R_CL value | Strap or RC choice | Hold / Latch / Retry | A | ms | None / OL / OC / OT | Free text | Pass / Adjust / Fail |

## CL and DELAY tuning matrix
Use this as the decision guide while selecting the first default population.

| Target behavior | CL objective | DELAY objective | Typical use on this project | Bench checks | Pass criteria |
| --- | --- | --- | --- | --- | --- |
| Conservative first power | Lowest safe current limit that still actuates test load | Fault behavior that is easy to observe and recover | Earliest bench work with unknown coil envelope | Confirm no abnormal heating and no runaway current | Stable output and predictable fault reporting |
| Game pulse validation | Current limit high enough for reliable pulse actuation margin | Behavior that prevents rapid uncontrolled retrigger after fault | Short pulse coils on S2..S6 | Confirm pulse completes and fault logic remains deterministic | No missed intended pulse and no sustained overcurrent |
| Fault-injection check | Current limit set to trigger under forced overload | DELAY behavior set so scope capture is easy | Deliberate short or overload simulation | Capture IN, OUT, and fault response timing | Device protects channel as expected |
| Thermal margin check | Current limit and pulse envelope representative of expected use | Behavior that avoids hidden thermal cycling | Repeated pulse bursts near worst-case duty | Check package and copper heating trend over time | Temperature trend remains acceptable |

## Minimum measurements per channel
- supply voltage at the channel input during pulse
- output waveform at the switched output node
- current waveform (or equivalent shunt/supply capture)
- fault signal behavior for at least one forced fault case

## Decision log
When a channel setting is accepted, copy the final choice here and mirror it into the schematic notes.

| Channel | Final CL population | Final DELAY population | Verified mode | Notes |
| --- | --- | --- | --- | --- |
| S2 | TBD | TBD | TBD | |
| S3 | TBD | TBD | TBD | |
| S4 | TBD | TBD | TBD | |
| S5 | TBD | TBD | TBD | |
| S6 | TBD | TBD | TBD | |

## Important
Do not treat this table as a datasheet replacement.

Use the latest TI datasheet equations, limits, and pin behavior definitions when selecting final CL and DELAY values.