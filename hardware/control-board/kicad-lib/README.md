# Local KiCad Library (Repo-Tracked)

Put third-party symbols and footprints for this board here so paths are stable and git-tracked.

## Folder layout
- `symbols/` -> `.kicad_sym` files
- `footprints/` -> `.pretty` folders containing `.kicad_mod` files
- `3dmodels/` -> optional `.step` or `.wrl` files

## Recommended SnapMagic import naming
- symbol library file: `symbols/Snapmagic_TPS1H200A_Q1.kicad_sym`
- footprint library folder: `footprints/Snapmagic_TI_TPS1H200A_Q1.pretty/`

## KiCad path setup
In KiCad, when adding the library paths, use project-variable form so links survive on other machines:
- symbol library path: `${KIPRJMOD}/kicad-lib/symbols/Snapmagic_TPS1H200A_Q1.kicad_sym`
- footprint library path: `${KIPRJMOD}/kicad-lib/footprints/Snapmagic_TI_TPS1H200A_Q1.pretty`

`KIPRJMOD` resolves to the folder that contains the `.kicad_pro` file.
