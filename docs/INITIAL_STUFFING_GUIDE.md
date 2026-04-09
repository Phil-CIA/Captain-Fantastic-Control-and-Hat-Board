# Initial Stuffing Guide for 5 V Firmware Bring-Up

Use this guide to decide what to populate now versus what can wait while the board is being brought up for **firmware development only**.

> This is a development-first recommendation, not the final production stuffing plan.

## Goal
Get the board to a safe state where the MCU and logic can be powered and tested on **5 V only**, while avoiding the risky **26 V solenoid-drive** condition until the `Vgs` issue is addressed.

## Populate now
These are the parts worth populating for the first bring-up pass:

- the **MCU / controller section** needed to boot firmware
- any **logic-side power path** needed for 5 V bring-up
  - 5 V input connection used for the bench supply
  - any regulator or local logic rail parts needed to generate the MCU supply
  - decoupling capacitors and basic power filtering
- **reset / boot / programming / debug** parts
- **USB / serial / header** parts needed to load firmware or observe boot logs
- **status LEDs** and their resistors, if they help bring-up
- input-side connectors or headers needed for safe logic testing
- the board-to-board connector parts needed to use the current **HAT-board workaround** during development
- test points or probe-friendly headers that help verify rails and GPIO activity

## Safe to leave off initially
If these parts are not needed for first firmware work, they can be left unpopulated for now:

- real **solenoid load connectors and field wiring**
- high-current output terminals that are not needed during logic bring-up
- optional indicator or convenience parts that do not affect boot, power, or debug access

## Recommended to leave off until the 26 V issue is controlled
For the safest first pass, avoid populating parts that only matter to the live high-voltage solenoid path unless you specifically need them mounted for fit-checking.

This especially includes the **solenoid-drive MOSFET chain** identified in the current schematic, such as:
- `IRFR5410` output devices in the solenoid path
- `BSS84` P-channel gate-drive devices associated with that path
- any other parts that exist only to switch or support the live **26 V** solenoid rail

## If the board is already fully stuffed
If those parts are already installed, the board can still be used for early firmware work **provided the 26 V supply remains disconnected** and the solenoid outputs are treated as inactive during bring-up.

Recommended precautions:
- power the board from a **current-limited 5 V bench supply only**
- do not connect real solenoid loads yet
- do not test live high-energy output switching yet
- monitor current draw and part temperature during the first boot

## Practical first-pass recommendation
### Best path for fastest safe bring-up
1. populate the **logic, MCU, power, and debug** sections first
2. leave the **26 V solenoid-drive section** out if practical
3. bring the board up on **5 V only**
4. verify boot, serial output, GPIO behavior, and any safe low-energy interfaces
5. return to the solenoid section only after the temporary workaround or redesign is defined

## Related docs
- `docs/BENCH_STUFFING_CHECKLIST.md`
- `docs/FIRST_POWERUP_CHECKLIST.md`
- `docs/REV_B_ACTION_LIST.md`
- `docs/NEXT_ITERATION.md`
