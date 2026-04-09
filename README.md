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
- `docs/` – project status, redesign notes, and restart / parking-lot notes

## Current status
- New focused repo created on **2026-04-09**
- The current KiCad files uploaded to GitHub for both boards have been merged into this cleaned repo baseline
- The next PCB change will likely **not** be the last one; the goal is to keep building and developing the hardware while collecting more real-world feedback
- Two redesign blockers are currently tracked:
  1. the **P-channel solenoid-drive section on the base board sees gate-drive conditions above the recommended `Vgs` range when used on the 26 V supply**
  2. the **HAT-board connector did not line up with the base board**, although there is a temporary development workaround

## Next steps
1. define a safe temporary workaround for the 26 V solenoid-drive `Vgs` problem
2. continue development using the current connector workaround while keeping the proper PCB alignment fix in the next revision
3. follow the tracked redesign checklist in `docs/REV_B_ACTION_LIST.md`
4. start the next board revision from this corrected baseline
