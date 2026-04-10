# Rev B Redesign Action List

This document turns the first hardware findings into a concrete action list for the next board revision.

## Goal
Build a corrected next revision of the **base control board** and **HAT board** that:
- safely supports the **26 V solenoid supply**
- mates correctly across the **board-to-board connector**
- preserves the split-board development approach unless a later architectural change is intentionally made

## Development-first approach
The next PCB revision may **not** be the final set of board changes. The current intent is to keep the hardware usable for active development, apply safe workarounds where needed, and then roll the validated fixes into the next board revision.

### Short-term development posture
- the **connector misalignment** has a workable temporary path for development, so it remains a tracked PCB fix but is not the only thing blocking current bench work
- the **solenoid-drive issue** needs a safe workaround first, because the present P-channel gate-drive arrangement is pushing `Vgs` above the recommended range
- any workaround used for development should be clearly marked as temporary and reviewed again before the next fabrication order

## Initial firmware bring-up plan (5 V only)
For the first build, the goal is **firmware development**, not live solenoid operation.

- assemble the board with the logic and control sections needed for MCU bring-up
- power the board with **5 V only** for the first bench sessions
- keep the **26 V solenoid supply disconnected** until the gate-drive / `Vgs` problem is controlled
- validate regulator behavior, MCU boot, GPIO activity, serial/debug access, and any non-solenoid interface logic first
- if output verification is needed, use safe low-energy indicators or measurement points instead of real solenoid loads
- use `docs/FIRST_POWERUP_CHECKLIST.md` as the step-by-step bring-up procedure

## Must-fix items

### 1) Base board solenoid-drive section
**Problem**
The current **P-channel solenoid-drive arrangement on the base board is pushing the MOSFET drivers above the recommended `Vgs` range when used on the 26 V supply**. This is the same core issue already identified on the matrix-board work.

**Short-term workaround path**
- treat the on-board 26 V solenoid drive as **unsafe until the `Vgs` condition is controlled**
- evaluate a temporary gate-to-source clamp / gate-drive limiting approach for development use, or bypass the on-board drive with an external known-good driver stage while bring-up continues
- document exactly what temporary workaround is used on the bench so it does not get mistaken for the final PCB fix

**Rev B actions**
- audit every part in the solenoid-drive path for **voltage rating**, **power dissipation**, and **transient margin**
- replace the current discrete PMOS driver approach with a **protected smart high-side switch architecture** for the next revision
- implement the control-board respin around a **single-channel per output** layout strategy for easier routing and debugging
- review gate-drive parts, pull resistors, clamp parts, and transient suppression together as one circuit, not as isolated parts
- explicitly eliminate the current P-channel `Vgs` overstress condition from the design
- confirm that the flyback / suppression strategy is correct for the solenoid load and expected kick energy
- cross-check the revised approach against the lessons already learned from the matrix-board design
- use `docs/SOLENOID_DRIVER_SELECTION.md` as the design-direction note before freezing the respin
- implement the repeated output cell from `docs/SMART_SWITCH_CHANNEL_BLOCK.md` for the five smart-switch solenoid channels

**Verification before ordering**
- every part in the solenoid path has an acceptable voltage margin above the real supply and transient conditions
- ERC passes with the revised parts and connectivity
- the final schematic review explicitly signs off the 26 V path

### 2) HAT-board connector alignment
**Problem**
The **HAT-board connector did not line up** with the base board, preventing correct mating.

**Short-term workaround path**
- keep using the current physical workaround for development and firmware bring-up
- document the workaround clearly so the next board revision fixes the actual root cause instead of relying on manual rework or special assembly steps

**Rev B actions**
- verify the exact mating connector pair, footprint orientation, and pin numbering on both boards
- confirm the connector datum and placement against the real mechanical stack-up, not just schematic intent
- check spacing, offset, and stack height in KiCad using board outlines and connector courtyard / 3D alignment
- add mechanical references or notes so the connector location is locked before fabrication
- if needed, move to a connector strategy that is easier to align and inspect during assembly

**Verification before ordering**
- 1:1 print or measurement check confirms pin and body alignment
- connector placement is reviewed on both PCBs at the same time
- any needed mechanical keepouts or mounting references are documented

## Supporting cleanup
- keep `hardware/control-board/` as the active base-board source
- keep `hardware/hat-board/` as the active HAT-board source
- keep old snapshots only under `hardware/control-board/archive/`
- capture any additional bench findings in `docs/NEXT_ITERATION.md`

## Exit criteria for Rev B schematic freeze
- the 26 V solenoid path is fully reworked and reviewed
- the base-board / HAT-board connector alignment is confirmed mechanically
- the active KiCad files are clearly identified
- open questions are reduced to non-blocking cleanup items only

## Notes
- Date started: **2026-04-09**
- Source of issues: first review of the received base board and HAT board hardware
