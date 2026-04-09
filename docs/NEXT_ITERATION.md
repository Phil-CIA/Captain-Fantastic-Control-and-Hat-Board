# Next Iteration / Parking Lot

This file is the parking lot for the control-board and HAT-board redesign.

## Current focus
- redesign the base-board **P-channel solenoid circuit**, because the current components cannot handle **26 V**
- fix the **HAT-board connector alignment** so it mates correctly with the base board
- keep only the control-board / HAT-board material needed for this effort
- identify the authoritative KiCad files for the next revision

## Known issues to document next
- Issue 1: the **P-channel solenoid circuit components on the base control board cannot handle the 26 V solenoid supply**; this is the same underlying circuit problem already seen on the matrix board.
- Issue 2: the **connector for the HAT board did not line up correctly** with the matching connection on the base board.

## Parking lot
- decide whether the next architecture remains split between a **base board** and a **HAT board**, or gets recombined later
- keep connector / pin-mapping notes near the hardware source of truth
- capture any assembly or mechanical fit problems as soon as they are observed
- use `docs/REV_B_ACTION_LIST.md` as the working checklist for the next schematic pass

## Notes log
- 2026-04-09: new focused repo created from the larger Captain Fantastic project so the control-board and HAT-board redesign can proceed cleanly.
- 2026-04-09: current KiCad files for both the control board and the HAT board were uploaded and folded into the cleaned repo baseline.
- 2026-04-09: clarified that the HAT board is a companion board created by splitting the original control board during development, and documented the first two must-fix redesign issues.
