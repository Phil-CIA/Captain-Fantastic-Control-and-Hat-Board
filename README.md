# Captain Fantastic Control and HAT Board

Focused hardware repo for the **Captain Fantastic Home Edition** base control board and its companion **HAT board**.

## Scope
Originally, the system was built around two main boards:
- the **matrix board**
- the **control board**

During development, the control section was split into two physical boards:
- the **base control board**
- the **HAT board**

The HAT board is **not** a revision of the control board. It is a companion board that was created when the original control board was split for development work.

> The separate lamp/switch matrix board work lives in the dedicated repo: `Pinball-Sw-Lamp-Matrix-`.

## Repo layout
- `hardware/control-board/` – current base control-board KiCad source for the next redesign pass
- `hardware/control-board/archive/` – older control-board snapshots and backup exports kept for reference
- `hardware/hat-board/` – companion HAT-board KiCad source
- `firmware/control-board/` – clean control-board firmware baseline for this repo
- `docs/` – project status, redesign notes, firmware migration notes, and bring-up checklists

## Current status
- New focused repo created on **2026-04-09**
- The current KiCad files uploaded to GitHub for both boards have been merged into this cleaned repo baseline
- A **.96-inch SSD1306-style I2C OLED** has also been added as a troubleshooting display for boot messages and local status during bring-up
- The next PCB change will likely **not** be the last one; the goal is to keep building and developing the hardware while collecting more real-world feedback
- Two redesign blockers are currently tracked:
  1. the **P-channel solenoid-drive section on the base board sees gate-drive conditions above the recommended `Vgs` range when used on the 26 V supply**
  2. the **HAT-board connector did not line up with the base board**, although there is a temporary development workaround

## Next steps
1. build the board for firmware development and, if needed for proof-of-life output checks, feed **both rails from a current-limited 5 V source** rather than the real 26 V supply
2. use `docs/INITIAL_STUFFING_GUIDE.md` and `docs/BENCH_STUFFING_CHECKLIST.md` to decide what to populate now versus later
3. follow the safe bring-up procedure in `docs/FIRST_POWERUP_CHECKLIST.md`
4. proceed with the **single-channel smart high-side** respin direction using `docs/SOLENOID_DRIVER_SELECTION.md`, `docs/SINGLE_CHANNEL_SOLENOID_LAYOUT.md`, and `docs/SMART_SWITCH_CHANNEL_BLOCK.md` while continuing the clean firmware migration in `firmware/control-board/`
