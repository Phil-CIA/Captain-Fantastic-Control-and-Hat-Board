# Rev B Morning Handoff Template

REV B HANDOFF START

Date: [YYYY-MM-DD]
Project: Captain Fantastic Control + HAT Rev B
Phase: Problem rectification
Primary Goal Today: [one clear outcome]

Current State Snapshot:
- Documentation scaffold is already in place and usable.
- Live tracker exists and is the main execution doc.
- Decisions log exists and holds both confirmed decisions and open slots.
- Connector electrical pin map is frozen.
- GPIO shift-register control mapping is captured.
- Mechanical verification has a docs-only Stage A path.
- Bench-only items are explicitly deferred until hardware access resumes.

What Was Completed Last Session:
1. [completed item]
2. [completed item]
3. [completed item]

Open Technical Decisions:
1. Final startup feedback-fix implementation path
2. Final USB backfeed protection path
3. Final connector mating MPN pair and stack-height sign-off
4. Final rail-enable polarity and boot-default freeze sign-off
5. Final HAT measurement/control semantic contract sign-off
6. Final regulator electrical contract sign-off
7. Final analog current limiting per channel strategy

Current Highest Risk:
- Buck converter feedback reference is unstable during power-up.
- Before the MPU enables the range-switch MOSFETs, the feedback node can float.
- That can let converter output rise toward Vin minus about 1.23 V.
- Revision intent is to make feedback valid from first power-on.

Bench Status:
- Bench access: [available / unavailable]
- If unavailable, keep these deferred:
  - rail-enable polarity verification
  - boot-default verification
  - INA channel-to-rail truth table
  - physical connector mating / stack verification
- If available, prioritize GPIO polarity and boot-default capture first.

Current Doc Truth Sources:
- REGULATOR_HAT_REV_WORK_TRACKER.md
- REGULATOR_HAT_REV_DECISIONS_LOG.md
- REGULATOR_SCHEMATIC_ISSUES.md
- HAT_SCHEMATIC_ISSUES.md
- CONSOLIDATED_REV_B_ISSUES.md
- GPIO_PINOUT.md
- MECHANICAL_STACK_VERIFICATION.md

Already Confirmed Decisions:
- Use separate kickoff doc plus live tracker.
- Treat regulator and HAT as one coordinated Rev B cycle.
- Freeze interface before layout edits.
- Use firmware naming:
  - control.* for commands
  - telemetry.* for measurements
  - _mv for millivolts
  - _present for boolean presence flags
- Shift-register control mapping:
  - SR_Data = GPIO3
  - SR_CLK = GPIO4
  - SR_Latch = GPIO1
- Connector electrical pin map is frozen with NC_Reserved on unused pins.

Best Next Off-Bench Actions:
1. Complete Stage A docs-only capture in MECHANICAL_STACK_VERIFICATION.md
2. Close the short regulator close list and record chosen decisions
3. Close the short HAT semantic interface wording
4. Choose a simple analog current limiting approach per channel
5. Sync any closed decisions into the tracker, decisions log, and consolidated issue list

What Not To Rebuild:
1. Do not recreate process docs.
2. Do not expand back into large worksheet-style notes.
3. Do not mark bench-gated items closed without hardware evidence.
4. Do not reopen broad repo exploration unless a specific unresolved decision requires it.

Execution Request:
1. Read the tracker and decisions log first.
2. Summarize the current stopping point in 5 bullets or less.
3. Give me a strict 5-step plan for today.
4. Keep only one active step at a time.
5. After each step, give pass/fail criteria.
6. End with a short closeout block for the next chat.

First Action To Start Now:
- [example: close one regulator-side design decision in simple prose]
- [example: finish Stage A mechanical capture checklist]
- [example: prepare bench-only polarity test order]

REV B HANDOFF END