# Solenoid Bring-Up Handoff — 2026-05-08

## Session Summary

**Primary Goal**: Validate Rev 1 control board TPS1H200A solenoid drivers for 22V coil rail.

**Status**: 4 of 5 solenoids working with TPS output stage. **Outhole (S2) requires relay workaround** due to current-limit constraint on TPS CL/ILIM pin.

---

## What Was Done This Session

### Hardware Changes
1. **Solenoid CL/ILIM resistors**: Stepped down from `1kΩ` → `510Ω` → `270Ω` → `150Ω` → `0Ω`
   - Progression: `~1.5A` → `~3A` → `~5.5A` → `~9A` → `device max`
   - Result: S2 (outhole) did not achieve sufficient force even with 0Ω (no current limit)

2. **GPIO5 strapping pin issue discovered**: GPIO5 is an ESP32 boot-time strapping pin pulled HIGH during reset, which would latch S5 (bumper) coil ON at boot. **Firmware fix applied**: reversed `pinMode`/`digitalWrite` order in `initSolenoids()` to avoid glitch during init. **Board hardware fix needed**: add 10kΩ pull-down from U5 (S5 channel) IN pin to GND.

3. **510Ω CL setting is current standard** on all five TPS1H200A channels. All solenoids connected to 22V main supply.

### Firmware Changes (Control-and-Hat-Board repo)

#### `firmware/control-board/include/solenoid_gpio_config.h`
- S2 (outhole) pulse width: `45ms` → `100ms` (extended for mapping/force validation)
- Other solenoids: unchanged (`45ms`, `45ms`, `40ms`, `40ms`)

#### `firmware/control-board/src/main.cpp`
1. **Single-channel test mode**: Output test loop now fires **only S2** repeatedly (not cycling through all 5)
   - Added: `constexpr uint8_t OUTPUT_TEST_SINGLE_SOLENOID = SOLENOID_S2`
   - Modified `runOptionalOutputTest()` to: `activeOutput = static_cast<int8_t>(OUTPUT_TEST_SINGLE_SOLENOID);`
   - Simplified serial debug output

2. **GPIO init order fix**: `initSolenoids()` now writes `digitalWrite(LOW)` **before** `pinMode(OUTPUT)` to avoid drive glitch during power-on

3. **Firmware flashed to COM5**: `captain_control_test` environment, build successful, uploaded successfully.

---

## Current Test State

**Firmware**: `captain_control_test` profile (TEST mode)
- Fires only S2 (outhole) repeatedly
- Pulse: `100ms` ON, `~750ms` idle between pulses
- Cycles endlessly, prints per pulse on serial

**Hardware**:
- All 5 solenoids connected to 22V main supply
- All 5 TPS1H200A channels set to `510Ω` CL/ILIM
- GPIO5 init fix applied (firmware-side; still need 10k pull-down on board for full fix)

**Test Results**:
- **S3, S4, S5, S6**: Working, audible clicks, reasonable force
- **S2 (outhole)**: Operating but insufficient force to eject ball reliably even at 100ms and with CL shorted (0Ω)
- **GPIO5 spurious firing**: Fixed at boot time with init order change

---

## Root Cause Analysis: S2 Outhole

TPS1H200A device specs:
- Max continuous current: **10A** (nominal)
- Max peak current: limited by coil impedance + supply

Observations:
- Even with `CL` shorted to GND (unlimited current), outhole did not achieve target force
- This suggests: **The outhole coil itself, the plunger mechanics, or the wiring path has an impedance/friction issue**, not a board current-limit issue

**Not the board. Likely coil/mechanical.**

---

## Next Steps: Outhole Workaround (Relay Bridge)

### The Plan
Replace TPS S2 output stage with a **24V automotive relay** driven from the same ESP32 GPIO.

**Wiring**:
```
GPIO23 ──────────► Relay coil + (via current-limiting resistor or direct)
GND  ─────────────► Relay coil -
+24V supply ──────► Relay contact COM
Relay contact NO ─► Outhole solenoid +
Outhole solenoid - ► GND
```

**Parts Needed**:
1. 24V DC automotive relay (SPST-NO or SPDT), 20A+ contact rating, **with mounting tab and socket harness** (surface-mount)
2. Flyback diode across relay coil (`1N4007`, cathode to coil +, anode to coil -)
3. Flyback diode across solenoid coil (likely already present on machine)
4. Fuse holder + fuse for +24V branch

**Why This Works**:
- Relay coil only draws ~50–80mA (well within TPS capability)
- Relay contacts deliver full 24V at unlimited current to the solenoid
- Relay delay (~5–10ms) is **irrelevant for outhole** — ball sits idle anyway
- No board revision required
- No firmware change required

### Parallel Work: Debug the Outhole Coil Path

While waiting for relay parts, optionally bench-test:
1. Disconnect outhole coil from board
2. Feed it direct 24V from supply for ~100ms pulse manually
3. Observe if it achieves target force
4. If yes: wiring/board path is the issue
5. If no: coil or mechanics is the issue

---

## Firmware Status: Future Changes

### To Revert After Relay Workaround Installed
Once relay is wired and tested:
1. Keep S2 pulse at **60–80ms** (relay has no current limit, so force is achievable)
2. Revert to **all-five-channel cycling** test mode for full system validation
3. Flash `captain_control_test` and verify all 5 solenoids fire in sequence

### Eventual Production Firmware
Switch to `captain_control_system` profile (non-test mode) once hardware bringup is complete. Will require:
- I2C slave implementation on matrix board (0x24 address)
- `onMatrixSwitchEdge()` callback to couple solenoid firing to switch closure
- Scoring state machine
- Audio integration

---

## Files Modified This Session

| File | Change |
|---|---|
| `firmware/control-board/include/solenoid_gpio_config.h` | S2 pulse: `45ms` → `100ms` |
| `firmware/control-board/src/main.cpp` | Single-channel S2 test mode + GPIO init order fix |

---

## Known Issues / Board Revision Candidates

1. **GPIO5 strapping pull-up at boot**: Add 10kΩ pull-down from U5 (S2 channel) IN pin to GND to suppress spurious S5 firing on reset
2. **S2 TPS output insufficient**: Either coil/mechanics weak, or board path has impedance. Relay workaround in place; debug coil path if time permits.

---

## Git Commits

- **Commit hash**: [will be updated after push]
- **Message**: `Solenoid bring-up: S2 to 100ms, single-channel test mode, GPIO5 init fix`
- **Files**: 
  - `firmware/control-board/include/solenoid_gpio_config.h`
  - `firmware/control-board/src/main.cpp`

---

## Resume Checklist

When you return:

- [ ] Relay parts ordered/on-hand
- [ ] Relay wired to GPIO23, TPS S2 output disabled/rerouted
- [ ] Flyback diodes installed on relay coil and solenoid coil
- [ ] Test: S2 fires reliably with relay (should achieve full force now)
- [ ] Revert firmware to all-five-channel cycling
- [ ] Reflash `captain_control_test` to COM5
- [ ] Verify all 5 solenoids cycle (S2 with relay, others with TPS)
- [ ] Document relay test results in new handoff
- [ ] Consider: Debug outhole coil direct-power test (optional bench work)
- [ ] Move to Priority 1: I2C slave implementation on matrix board

---

## Contact Points

- **ESP32 Control Board**: COM5 (baud 115200)
- **Matrix Board**: COM4 (baud 115200)
- **Current Firmware**: `captain_control_test` (single-channel S2 mode, 100ms pulse)
- **Current Pulse Timing**: S2 fires, ~750ms idle, repeats

---

**Session End**: 2026-05-08 (session paused for location change)  
**Next Session**: TBD (after relay procurement and installation)
