# First Power-Up Checklist (5 V Only)

Use this checklist for the **first firmware-development bring-up** of the base control board and HAT board.

> Goal: confirm that the logic side of the hardware is safe and usable before any 26 V solenoid work is attempted.
>
> If you want the stripped-down version, use `docs/NEXT_STEP_BENCH_PLAN.md` for the one-step-at-a-time bench plan.
>
> For the newly ordered Rev 1 hardware arrival workflow, use `docs/REV1_POWERUP_ON_ARRIVAL.md`.

## 1) Before power is applied
- use `docs/INITIAL_STUFFING_GUIDE.md` and `docs/BENCH_STUFFING_CHECKLIST.md` to confirm the intended first-pass population set
- visually inspect the PCB for solder bridges, missing parts, wrong polarity parts, and connector orientation mistakes
- confirm that the current development workaround for the HAT-board connector is mechanically stable enough for bench use
- make sure the **real 26 V solenoid supply is fully disconnected**
- if practical, unplug or leave unstuffed any parts that are only needed for live solenoid drive testing
- set the bench supply to **5 V** with a conservative current limit for first power-up; if output proof-of-life is desired, feed both rails from that same 5 V source only

## 2) Passive sanity checks
- check for shorts between **5 V and GND** with a meter before applying power
- check that there is no accidental continuity from the 26 V rail into logic rails
- verify regulator input/output pins are not shorted
- verify the MCU power pins and main decoupling rails look sane on continuity checks

## 3) First 5 V power-up
- power the board from the current-limited **5 V** bench supply only
- if you want to observe the output-stage LEDs or proof-of-life switching, feed the output rail from that same **5 V** source instead of the real 22–26 V rail
- watch for unexpected current draw, heating, smoke, or parts warming immediately
- confirm the expected logic rail(s) come up to the proper voltage
- verify the MCU boots and any status LED / serial console behavior appears as expected

## 4) Firmware bring-up checks
- confirm USB / serial programming or debug access works
- watch the boot log for the I2C scan and the likely debug OLED detection report
- test basic firmware functions such as boot messages, GPIO toggling, and input reads
- verify any non-solenoid outputs using safe indicators, test points, or a meter / scope instead of real loads
- keep notes on what works, what is unstable, and what needs rework

## 5) Stop conditions
Stop immediately if any of the following occur:
- unexpected high current draw
- a regulator output is incorrect
- any part heats up abnormally on 5 V only
- any evidence appears that the 26 V path is coupling into the logic side unexpectedly

## 6) After first power-up
- record the measured rail voltages and current draw
- capture firmware bring-up status in `docs/NEXT_ITERATION.md`
- leave the 26 V solenoid path disabled until the gate-drive / `Vgs` issue has a defined safe workaround or redesign
- when starting TPS1H200 channel validation, log CL and DELAY experiments in `docs/TPS1H200_CL_DELAY_BENCH_TABLE.md`
