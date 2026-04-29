# Machine Transfer Handoff (2026-04-30)

Purpose: resume from today's verified firmware state and execute the next bench step with a tracked plan.

## Where we are right now
- Matrix/display board (`COM4`) upload path is confirmed and working.
- Control board (`COM5`) upload path is confirmed and working.
- Control board firmware now self-identifies at boot with version/build/signature:
  - `FW: control-board | build=<env>-<unix_time> | sig=<hex> | built=<date time>`
- TEST profile on control board was flashed and validated:
  - profile prints `Profile: TEST`
  - output test cycles `S2..S6` in serial output
- Matrix link may still be degraded at boot; TEST output cycling remains enabled in that condition.

## Safety boundary for morning bench
- Use 5V current-limited bench power for LED/logic validation.
- Keep real 26V coil rail disconnected.
- Validate visible outputs first, then command-path behavior.

## Tracked morning plan
1. Power and baseline checks
- Connect control board and matrix/display board.
- Confirm ports (`COM4` matrix/display, `COM5` control).
- Open control serial monitor and capture boot identity line.

2. Validate output activity with known-good TEST profile
- Confirm repeating `Output test -> S2..S6` lines.
- Confirm corresponding LED/driver indicators on 5V bench setup.

3. Playfield light control path test (requested focus)
- Keep control board online and matrix/display board connected.
- Move from autonomous test pulsing to commanded light behavior validation.
- Capture which commands/states map to which observed playfield lamps.

4. Stabilize and document
- Record pass/fail per lamp group.
- Record any non-responsive channels and wiring notes.
- Decide if we stay in TEST or reflash SYSTEM at end of session.

## Known-good commands
From `firmware/control-board`:

```powershell
C:\Users\user\.platformio\penv\Scripts\platformio.exe run -e captain_control_test --target upload --upload-port COM5
C:\Users\user\.platformio\penv\Scripts\platformio.exe device monitor --port COM5 --baud 115200
```

From matrix/display firmware folder:

```powershell
C:\Users\user\.platformio\penv\Scripts\platformio.exe run -e display_board --target upload --upload-port COM4
```

## Notes for first message tomorrow
- Priority is playfield-light control from control board, not just autonomous test cycling.
- Keep updates concise with direct hardware-observation checkpoints.
- Continue using firmware identity line to verify exactly what is running before each test.
