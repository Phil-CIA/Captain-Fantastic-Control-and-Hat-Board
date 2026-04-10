# TPS1H200 Schematic Conversion Worksheet

This worksheet is the direct bridge from the current discrete solenoid block to five repeated `TPS1H200A-Q1` channels.

Use this during an interactive KiCad schematic edit session.

## Scope
- Replace the old discrete solenoid drive path centered on `Q10`, `Q11`, and `Q18`
- Implement five channels for `S2`..`S6`
- Keep firmware-facing control naming aligned with `SD2`..`SD6`

## Existing anchor points in the current schematic
- old references:
  - `Q11` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L33903)
  - `Q10` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L41791)
  - `Q18` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L42519)
- control/output rail labels already present:
  - `SD2` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L11548)
  - `SD3` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L11658)
  - `SD4` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L12252)
  - `SD5` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L12164)
  - `SD6` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L11944)
  - `S2` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L12208)
  - `S3` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L12032)
  - `S4` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L11636)
  - `S5` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L11900)
  - `S6` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L12010)
  - `+22vdc` at [hardware/control-board/Pinball Control board.kicad_sch](hardware/control-board/Pinball%20Control%20board.kicad_sch#L11592)

## Channel mapping to apply
| Channel | Control net | Output net | Supply net |
| --- | --- | --- | --- |
| U_S2 | SD2 | S2 | +22vdc |
| U_S3 | SD3 | S3 | +22vdc |
| U_S4 | SD4 | S4 | +22vdc |
| U_S5 | SD5 | S5 | +22vdc |
| U_S6 | SD6 | S6 | +22vdc |

## Required per-channel support items
- local decoupling capacitor at device supply pin
- CL programming population (value per bench tuning)
- DELAY configuration population (mode per bench tuning)
- FAULT routing to test point and optional MCU net

## Execution checklist
1. Add/import `TPS1H200A-Q1` symbol to the project schematic library.
2. Place five channel symbols (`U_S2`..`U_S6`) near existing solenoid output region.
3. Rewire control nets: `SD2`..`SD6` to channel inputs.
4. Rewire outputs: channel outputs to `S2`..`S6`.
5. Rewire supply and return: `+22vdc` and ground per channel.
6. Add CL, DELAY, and FAULT support components per channel.
7. Remove legacy discrete path parts no longer used (`Q10`, `Q11`, `Q18`, and dependent gate-drive parts).
8. Run ERC and resolve all new issues.
9. Mirror chosen CL/DELAY settings in `docs/TPS1H200_CL_DELAY_BENCH_TABLE.md`.

## Note on tooling
The project currently does not include a dedicated `.kicad_sym` library in-repo, so symbol insertion is best done interactively in KiCad to avoid malformed schematic edits.

## SnapMagic import and repo sync steps
Use this flow to keep your local KiCad setup and git repo aligned.

1. Copy your downloaded SnapMagic symbol file into:
  - `hardware/control-board/kicad-lib/symbols/`
2. Copy your downloaded SnapMagic footprint `.pretty` folder into:
  - `hardware/control-board/kicad-lib/footprints/`
3. In KiCad Schematic Editor, open **Preferences -> Manage Symbol Libraries**.
4. Add your symbol library as **Project Specific** and paste this in the path/address box:
  - `${KIPRJMOD}/kicad-lib/symbols/<your_symbol_file>.kicad_sym`
5. In KiCad PCB Editor (or Preferences), open **Manage Footprint Libraries**.
6. Add your footprint library as **Project Specific** and paste this in the path/address box:
  - `${KIPRJMOD}/kicad-lib/footprints/<your_footprint_library>.pretty`
7. Save library tables and place the symbol in schematic.
8. In terminal, run:
  - `git add hardware/control-board/kicad-lib`
  - `git add hardware/control-board/Pinball Control board.kicad_sch`
  - `git add hardware/control-board/Pinball Control board.kicad_pcb`
  - `git commit -m "Add project-local TPS1H200 symbol and footprint libraries"`
  - `git push`

This keeps your symbol and footprint paths portable and tracked in the repo.