# Next Step Bench Plan

This is the **simple, one-thing-at-a-time** plan for the current control-board work.

> For now, **do not apply the real `+22vdc` / `26 V` solenoid supply yet**. First prove the logic side and the board basics on current-limited `5 V` only.

## Where we are now
- the solenoid-driver direction is now **`TPS1H200A-Q1`**
- the repeated channels for `S2`..`S6` have been duplicated into the schematic
- the project-local KiCad symbol / footprint / 3D model files are now in the repo
- the latest schematic checkpoint has already been saved to GitHub

## Immediate next step

### 1) Meter checks with the board unpowered
- check `5V` to `GND` for a short
- check `+22vdc` to `GND` for a dead short
- check there is **not** accidental continuity between the `+22vdc` rail and the `5V` logic rail
- if anything looks wrong here, stop before applying power

### 2) First power-up on `5V` only
- set the bench supply to `5V`
- use a conservative current limit
- leave the real coil / solenoid supply disconnected
- watch for unexpected current draw, hot parts, smoke, or smell
- confirm the expected logic rail behavior and MCU boot behavior

### 3) If the `5V` test behaves normally
- validate **one `TPS1H200A-Q1` channel at a time**
- keep the current `CL` resistor value as a **placeholder** until the bench proves it
- keep `DELAY` in the simple non-auto-retry direction for now
- record what happens in `docs/TPS1H200_CL_DELAY_BENCH_TABLE.md`

## What is coming after this
- finalize the `CL` resistor value and any `DIAG_EN` default choice
- clean up any last schematic details after bench learning
- carry the repeated smart-switch pattern into the PCB / layout work
- move into controlled coil testing and then wider board bring-up
- keep the firmware / HAT-board workflow moving in parallel

## Simple roadmap
- **Now:** safe meter checks and `5V`-only bring-up
- **Next:** one-channel smart-switch validation
- **Then:** freeze support values and clean up schematic / PCB details
- **After that:** next board-spin decisions and broader system integration

## Focus note
You do **not** need to hold the whole project in your head at once.

The only thing to focus on right now is: **safe bench checks first, one step at a time**.
