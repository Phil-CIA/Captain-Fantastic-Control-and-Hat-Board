# Captain Fantastic Control and HAT Board

Focused hardware repo for the **Captain Fantastic Home Edition** control board redesign and the related **HAT board** work.

## Scope
This repo is intended to hold only the files needed for:
- the main **control board**
- the legacy / reference **HAT board**
- redesign notes, issues, and next-iteration planning

> The separate lamp/switch matrix board work lives in the dedicated repo: `Pinball-Sw-Lamp-Matrix-`.

## Repo layout
- `hardware/control-board/` – current control board KiCad source copied from the main Captain repo
- `hardware/hat-board/` – legacy / reference HAT board KiCad source
- `docs/` – project status, redesign notes, and restart / parking-lot notes

## Current status
- New focused repo created on **2026-04-09**
- Existing hardware source has been copied in as a starting point
- Immediate next step is to document the **two known redesign blockers** found during first board arrival / inspection

## Next steps
1. capture the current hardware problems clearly
2. clean up repo structure and remove junk / stale exports as needed
3. decide whether the HAT board is a reference-only design or part of the next active revision
4. start Rev B / redesign planning from the cleaned baseline
