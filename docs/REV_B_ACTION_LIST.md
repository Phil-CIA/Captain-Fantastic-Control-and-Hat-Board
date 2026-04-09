# Rev B Redesign Action List

This document turns the first hardware findings into a concrete action list for the next board revision.

## Goal
Build a corrected next revision of the **base control board** and **HAT board** that:
- safely supports the **26 V solenoid supply**
- mates correctly across the **board-to-board connector**
- preserves the split-board development approach unless a later architectural change is intentionally made

## Must-fix items

### 1) Base board solenoid-drive section
**Problem**
The current **P-channel solenoid circuit components cannot handle 26 V**. This is the same core issue already identified on the matrix-board work.

**Rev B actions**
- audit every part in the solenoid-drive path for **voltage rating**, **power dissipation**, and **transient margin**
- replace any under-rated P-channel devices and support parts with components that are appropriate for the real **26 V** operating environment
- review gate-drive parts, pull resistors, clamp parts, and transient suppression together as one circuit, not as isolated parts
- confirm that the flyback / suppression strategy is correct for the solenoid load and expected kick energy
- cross-check the revised approach against the lessons already learned from the matrix-board design
- clearly mark the solenoid-drive section in the schematic for review before routing

**Verification before ordering**
- every part in the solenoid path has an acceptable voltage margin above the real supply and transient conditions
- ERC passes with the revised parts and connectivity
- the final schematic review explicitly signs off the 26 V path

### 2) HAT-board connector alignment
**Problem**
The **HAT-board connector did not line up** with the base board, preventing correct mating.

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
