# Changelog

All notable changes to this repo will be documented here.

## [Unreleased]
### Added
- initial focused repo scaffold for the Captain Fantastic control board and HAT board redesign
- baseline control-board and HAT-board KiCad sources copied from the main Captain Fantastic repo
- project status and next-iteration parking-lot notes
- `docs/REV_B_ACTION_LIST.md` with the first concrete redesign checklist for the 26 V solenoid issue and HAT-board connector alignment issue
- `docs/FIRST_POWERUP_CHECKLIST.md` for the staged 5 V-only firmware bring-up process
- `docs/INITIAL_STUFFING_GUIDE.md` with a practical populate-now vs populate-later recommendation for safe first bring-up
- `docs/BENCH_STUFFING_CHECKLIST.md` as a short assembly-side checklist for the first 5 V bring-up build
- `docs/FIRMWARE_MIGRATION.md` plus a clean `firmware/control-board/` baseline to start merging the old control firmware into this repo
- `docs/OLED_DEBUG_DISPLAY.md` and SSD1306-oriented boot-time I2C/OLED support in the new firmware baseline for the added .96-inch troubleshooting display
- `docs/SOLENOID_DRIVER_SELECTION.md` documenting the current driver-direction decision: move away from the discrete PMOS stage and toward a smart high-side switch for the respin
- `docs/SINGLE_CHANNEL_SOLENOID_LAYOUT.md` locking in the easier single-channel layout approach for the five control-board solenoid outputs

### Changed
- merged the current uploaded KiCad files for both boards into the organized `hardware/` layout
- archived older control-board snapshot files under `hardware/control-board/archive/`
- clarified in the docs that the HAT board is a companion board created by splitting the original control board during development, not a revision of it
- documented the first two must-fix redesign issues: the 26 V solenoid-driver problem and the HAT-board connector misalignment
- updated the plan to reflect a development-first approach, including the connector workaround and the `Vgs` concern on the solenoid-drive path
- added the staged **5 V only** bring-up plan for initial firmware development with the 26 V solenoid rail left disconnected
