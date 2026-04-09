# Bench Stuffing Checklist

Use this as a quick assembly-side checklist during the first bring-up build.

## Populate first
- [ ] MCU / controller module or related support parts
- [ ] 5 V input path
- [ ] regulator / logic rail parts needed for MCU power
- [ ] decoupling capacitors around power and MCU sections
- [ ] reset / boot / programming / debug headers
- [ ] status LEDs and their resistors
- [ ] board-to-board connector parts needed for the current HAT-board workaround
- [ ] any test points or headers useful for probing rails and GPIO

## Leave off for now if possible
- [ ] 26 V solenoid supply connection
- [ ] solenoid load wiring
- [ ] high-current output terminals not needed for logic bring-up
- [ ] optional parts that do not affect boot, power, or debug access

## Avoid energizing during first bring-up
- [ ] `IRFR5410` solenoid output path kept out of service
- [ ] `BSS84` gate-drive path treated as not yet validated for 26 V use
- [ ] no real solenoid loads connected

## Before first power
- [ ] inspect for solder bridges and polarity mistakes
- [ ] verify no short between 5 V and GND
- [ ] confirm 26 V rail is disconnected
- [ ] set bench supply to 5 V with a current limit

## Bring-up goal
- [ ] boot firmware successfully
- [ ] verify serial/debug access
- [ ] check LEDs / GPIO / safe logic behavior
- [ ] record anything odd in `docs/NEXT_ITERATION.md`
