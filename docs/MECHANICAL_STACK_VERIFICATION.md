# Mechanical Stack Verification (Rev B)

## Purpose

Verify connector alignment and stack-up fit before ordering boards.

## Execution Mode

- Stage A (off-bench): CAD and documentation checks that can be completed now.
- Stage B (physical): 1:1 print and hardware fit checks when physical access is available.

## Workspace Constraint Note

- Current workspace snapshot is documentation-focused and does not include the KiCad source files.
- Stage A can still be completed by importing evidence from the CAD workstation (screenshots, datasheet links, and measured notes).

## Required Inputs

- Latest regulator PCB print (1:1)
- Latest HAT PCB print (1:1)
- Connector datasheets and 3D models
- Final board outlines and keepouts

## Checklist

| Step | Check | Status | Notes |
|---|---|---|---|
| 1 | Confirm mating connector part numbers match intended pair. | Open | Connector refs are frozen to J1/J2, but exact mating MPN pair is still TBD. |
| 2 | Confirm footprint orientation and pin-1 marker alignment on both boards. | In Progress | Orientation check path defined; awaiting CAD capture artifacts. |
| 3 | Overlay 1:1 prints and verify pin/body alignment. | Pending Physical | Complete when print/physical assets are available. |
| 4 | Verify connector courtyard and body clearances in PCB view. | In Progress | Active KiCad source paths are identified; CAD evidence capture still required. |
| 5 | Verify stack height and any standoff constraints. | Open | Needs connector MPN pair before datasheet stack-up can be closed. |
| 6 | Verify keepouts around connector and tall components. | In Progress | Review criteria defined in Rev B actions; CAD evidence capture still required. |
| 7 | Record pass/fail and any required footprint/layout changes. | Open | Fill after Stage A and Stage B checks. |

## Evidence Log

| Date | Stage | Artifact | Result | Owner | Notes |
|---|---|---|---|---|---|
| 2026-05-16 | Stage A | Source-of-truth paths identified (`hardware/control-board/`, `hardware/hat-board/`) | Pass | You | From PROJECT_STATUS.md current-state section. |
| 2026-05-16 | Stage A | Connector electrical map + NC disposition frozen | Pass | You | From CONNECTOR_PINOUT_MAP.md freeze state and NC disposition sections. |
| 2026-05-16 | Stage A | Mechanical review criteria captured | Pass | You | From REV_B_ACTION_LIST.md HAT-board connector alignment actions. |
| TBD | Stage A | Connector datasheet links and MPN pair list | Open | You | Still needed to close part-number and stack-height checks. |
| TBD | Stage A | CAD screenshot of pin-1 orientation on both boards | Open | You | Capture once KiCad board views are reviewed side-by-side. |
| TBD | Stage A | CAD clearance check notes | Open | You | Capture courtyard/body clearance findings and any violations. |
| TBD | Stage B | 1:1 print overlay photo or measurement sheet | TBD | You | |
| TBD | Stage B | Stack-height fit photo/measurement notes | TBD | You | |

## Stage A Capture Protocol (Docs-Only)

1. From the CAD workstation, export or capture these artifacts:
	- board view screenshot for regulator connector area with pin-1 marker visible
	- board view screenshot for HAT connector area with pin-1 marker visible
	- board view screenshot showing connector courtyard/body clearances and nearby keepouts
	- connector datasheet links and exact mating MPN pair
2. Save artifact names in Notes using this naming pattern:
	- 2026-05-16_revB_j1_pin1_orientation.png
	- 2026-05-16_revB_j2_pin1_orientation.png
	- 2026-05-16_revB_connector_clearance.png
3. Update Evidence Log Result fields to Pass/Fail for each captured artifact.
4. Update Mechanical Freeze Sign-Off statuses only after corresponding artifact rows are filled.

## Stage A Exit (Before Stage B)

Stage A is considered complete when all are true:
- Connector datasheet links and mating MPN pair are recorded.
- Pin-1 orientation evidence exists for both boards.
- Courtyard/keepout CAD evidence is recorded with pass/fail notes.

## Stage A Baseline Findings (Doc-Backed)

1. Active design sources are identified for both boards and treated as authoritative.
2. Connector electrical pin-function map and NC disposition are already frozen for this revision.
3. Remaining mechanical blockers are narrowed to:
	- exact mating MPN pair selection
	- pin-1/orientation CAD evidence
	- courtyard and stack-height closure

## Mechanical Freeze Sign-Off

| Item | Status | Notes |
|---|---|---|
| Connector part-number pair frozen | Open | MPN pair not yet documented. |
| Pin-1 orientation verified (schematic + PCB) | In Progress | Verification workflow defined; CAD artifacts pending. |
| 1:1 print/physical alignment verified | Pending Physical | |
| Stack-height constraints verified | Open | Requires connector MPN dimensions and physical stack assumptions. |
| Keepout and clearance review complete | In Progress | Criteria defined; CAD evidence capture pending. |

## Exit Criteria

1. Alignment check passes on both paper overlay and CAD review.
2. Any offsets are corrected in both board layouts.
3. Mechanical notes are reflected in the decisions log.
4. Mechanical Freeze Sign-Off table is fully complete.

## Related Docs

- CONNECTOR_PINOUT_MAP.md
- REGULATOR_HAT_REV_DECISIONS_LOG.md
- REGULATOR_HAT_REV_WORK_TRACKER.md
