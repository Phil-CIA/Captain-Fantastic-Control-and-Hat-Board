# Changelog

All notable changes to this repo will be documented here.

## [Unreleased]
### Added
- initial focused repo scaffold for the Captain Fantastic control board and HAT board redesign
- baseline control-board and HAT-board KiCad sources copied from the main Captain Fantastic repo
- project status and next-iteration parking-lot notes

### Changed
- merged the current uploaded KiCad files for both boards into the organized `hardware/` layout
- archived older control-board snapshot files under `hardware/control-board/archive/`
- clarified in the docs that the HAT board is a companion board created by splitting the original control board during development, not a revision of it
- documented the first two must-fix redesign issues: the 26 V solenoid-driver problem and the HAT-board connector misalignment
