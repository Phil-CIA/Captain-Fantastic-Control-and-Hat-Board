# Firmware Integration Handoff — 2026-04-29

**Status**: Ready for Priority 1 Implementation  
**Handoff from**: Initial discovery and CI fix phase  
**Handoff to**: Firmware integration implementation  

---

## 1. Project Overview

This is a **migration of proven pinball machine behavior** across three hardware generations:
1. **Captain Fantastic home-edition** (original, proved core machine behavior + audio, but audio had switching noise under MOSFET drive)
2. **Control-board v1 PMOS redesign** (attempted to fix audio with higher voltage rails, but gate-drive stage exceeded Vgs limits; components damaged at 26V stress)
3. **New smart-switch boards** (current direction: TPS1H200A single-channel solenoid drivers + matrix board with VNQ7E100AJTR rows + LMV393 comparators + 74HC595 lamp columns)

**Core Goal**: Consolidate proven code from all three boards into one cohesive firmware set that runs on the new smart-switch hardware without reinventing already-solved behaviors (audio effects, matrix scanning, OTA firmware updates, I2C register protocols).

---

## 2. Current State Summary

### Control Board Firmware (~80% complete, structure-ready)
**Location**: `Captain-Fantastic-Control-and-Hat-Board/firmware/control-board/`  
**Framework**: Arduino, ESP32 dev kit, PlatformIO

| Component | Status | Notes |
|-----------|--------|-------|
| **main.cpp** | ✅ Scaffolded | FreeRTOS runtimes instantiated, loop structure correct, all subsystems wired |
| **I2C master polling** | ✅ Working | matrix_interface_runtime.cpp polls 0x24 every 20ms, link health tracking active |
| **Headbox 595 lamp drive** | ✅ Proven | Shift-register output for scoring display + attract mode, tested |
| **Solenoid GPIO config** | ✅ Locked | 5 channels (S2–S6 on GPIO 23, 19, 18, 5, 17), pulse timings defined |
| **Protocol contract** | ✅ Locked | HT16K33-style register model (0x00–0x07 lamps, 0x40–0x43 switches, 0xF0–0xF3 diag) |
| **Switch/lamp naming** | ✅ Complete | captain_mapping.h has all 32 switches + 40 lamps, GPIO bit lookups verified |
| **Game logic** | ❌ **STUB** | onMatrixSwitchEdge() callback is empty; no solenoid coupling, no scoring, no lamp state |
| **Audio system** | ⚠️ Infrastructure | MP3 queue + playback infrastructure present, not wired to game events |

**Key Files**:
- [src/main.cpp](../../firmware/control-board/src/main.cpp) — entry point, main loop
- [src/input/matrix_interface_runtime.cpp](../../firmware/control-board/src/input/matrix_interface_runtime.cpp) — I2C master to 0x24
- [src/headbox/headbox_runtime.cpp](../../firmware/control-board/src/headbox/headbox_runtime.cpp) — 595 lamp drive
- [include/captain_protocol.h](../../firmware/control-board/include/captain_protocol.h) — register map (LOCKED)
- [include/captain_mapping.h](../../firmware/control-board/include/captain_mapping.h) — switch/lamp names
- [include/solenoid_gpio_config.h](../../firmware/control-board/include/solenoid_gpio_config.h) — GPIO pin assignments

### Matrix Board Firmware (bring-up complete, **I2C slave missing**)
**Location**: `Pinball-Sw-Lamp-Matrix-/firmware/src/`  
**Framework**: ESP-IDF, ESP32-C6, native C++

| Component | Status | Notes |
|-----------|--------|-------|
| **Shift-register lamp drive** | ✅ Proven | sr_compose_frame() and sr_shift_frame() working, tested on hardware |
| **GPIO switch scan** | ✅ Proven | 5ms row advance, 8×4 matrix, per-row hit counters |
| **OLED debug display** | ✅ Working | Status telemetry, link activity bars |
| **I2C slave at 0x24** | ❌ **BLOCKER** | GPIO2/3 reserved in comments, not implemented; no i2c_slave.h, no register handlers |
| **Attract mode** | ✅ Proven | 24-step lamp sequence, 120ms cadence, ready for game logic |

**Key Files**:
- [src/main.cpp](../../Pinball-Sw-Lamp-Matrix-/firmware/src/main.cpp) — entry point, shift-reg + GPIO scan + OLED

### CI Build Status
**Status**: ✅ FIXED  
**Change**: Removed `platform_packages` line from platformio.ini (was pinning ESP-IDF version not resolvable on GitHub runners)  
**Verification**: GitHub Actions run 26+ should pass

---

## 3. Blocker Analysis & Priority Roadmap

### Priority 1 (BLOCKER): Add I2C Slave to Matrix Board
**Failure mode without this**: Control board cannot communicate with matrix board; matrix polling hangs; system initialization stalls.

**Implementation**:
- **File**: `Pinball-Sw-Lamp-Matrix-/firmware/src/main.cpp`
- **Scope**: 
  - Include `driver/i2c_slave.h`
  - Configure slave at address 0x24 on GPIO2 (SDA) / GPIO3 (SCL)
  - Implement register handlers for:
    - **0x00–0x07** (write-only): Lamp rows (8 bytes, one per row, bit=1 = lamp ON, active-high in register)
    - **0x40–0x43** (read-only): Switch snapshot (4 bytes, 32 switches packed, bit=1 = switch closure)
    - **0xF0–0xF3** (read-only): Diagnostics flags (system enabled, output enabled, test mode, live sample count)
  - Update register handler on every I2C write/read to stay in sync with real-time state
- **Acceptance**: 
  - Control board serial log shows `matrix detected = true`
  - Control board serial log shows `linkHealthy = true` (after 1 second)
  - Matrix board acknowledges write to 0x20 (system enable command) with register reflection

**Register Protocol (v1, Locked)**
```
Command writes:
  0x20 | 0x01  → Enable system (register 0xF0 bit 7 = 1)
  0x80 | 0x01  → Enable outputs (register 0xF0 bit 6 = 1)
  0xE0–0xEF    → Pulse level (0–15, used for diagnostic output pulse width)

Reads:
  0x00–0x07    → Lamp row state (mirror of last write)
  0x40–0x43    → Live switch snapshot (real-time scan result)
  0xF0         → System flags: bit7=sys_en, bit6=out_en, bit5–2=test_flags, bit1–0=diag_mode
```

---

### Priority 2: Implement Solenoid Coupling (Game Logic Entry Point)
**Blocker removed by**: Priority 1 (I2C slave working)  
**Dependency**: Matrix board responding to 0x24 reads; switch edges flowing through matrix_interface_runtime.cpp

**Implementation**:
- **File**: `Captain-Fantastic-Control-and-Hat-Board/firmware/control-board/src/main.cpp`
- **Location**: Search for `onMatrixSwitchEdge()` callback (currently empty stub)
- **Scope**:
  - Wire switch closures to solenoid pulses per machine rules:
    - Inlane switch (S1) → Inlane solenoid (S3)
    - Outlane switch (S20) → Outhole solenoid (S2)
    - Bumper switches → Bumper solenoid (S5) with optional multi-hit
    - Slingshot switches → Slingshot solenoids (S4 or S6)
  - Use `triggerSolenoid(SOLENOID_NAME)` from solenoid_gpio_config.h
  - Respect CAPTAIN_SOLENOID_PULSE_MS timing (45ms for S2/S3, 40ms for S4/S5/S6)
- **Acceptance**:
  - Physical switch press triggers audible solenoid click
  - Solenoid pulse lasts correct duration (verified on oscilloscope or by ear)
  - No double-triggers within 200ms (debounce window in matrix_interface_runtime.cpp)

---

### Priority 3: Scoring & Lamp State Machine
**Blocker removed by**: Priority 2 (solenoid coupling working; game events flowing)  

**Implementation**:
- **Scope**:
  - Track game state: score, player number, ball number, bonus multiplier
  - On solenoid trigger, increment score
  - Update lamp rows 0x00–0x07 on I2C write (score display, player indicator, ball indicator)
  - Update headbox 595 lamps (captain::headbox::Runtime already has setLamp/writeLamps)
- **Acceptance**:
  - Score increments on solenoid trigger
  - Lamp rows reflect score state on next I2C poll (20ms latency acceptable)

---

### Priority 4: Audio Parity Migration
**Dependency**: Priorities 1–3 working; game events flowing

**Implementation**:
- **Scope**:
  - Audio infrastructure (MP3 queue, playback) is already in control-board src/audio/
  - Wire solenoid trigger events to audio playback (bumper hit → bumper sound, etc.)
  - Migrate sound effects from home-edition or prior boards
- **Acceptance**:
  - Bumper hit triggers bumper sound effect without lag
  - No audio noise contamination on solenoid rail (validate with oscilloscope on 26V rail)

---

## 4. Technical Contracts & Locked Definitions

### I2C Register Map (Canonical)
| Address Range | Direction | Purpose | Notes |
|---|---|---|---|
| 0x00–0x07 | Write-only (control→matrix) | Lamp row state | 8 bytes, one per row; bit=1 ⇒ lamp ON (active-high) |
| 0x40–0x43 | Read-only (matrix→control) | Switch snapshot | 4 bytes packed; bit=1 ⇒ switch closure (active-high) |
| 0xF0–0xF3 | Read-only (matrix→control) | Diagnostics flags | System enable, output enable, test flags, live sample counts |
| 0x20, 0x80, 0xE0–0xEF | Command (control→matrix) | System control | Enable sys, enable outputs, set pulse levels |

**I2C Parameters**:
- Slave address: **0x24**
- GPIO (matrix board): **GPIO2 (SDA), GPIO3 (SCL)**
- GPIO (control board): **GPIO21 (SDA), GPIO22 (SCL)** (main I2C bus, shared with OLED)
- Frequency: **100 kHz**
- Poll interval (control→matrix): **20 ms**
- Link timeout: **1000 ms** (if no heartbeat in 1s, linkHealthy = false)

### Hardware Configuration

#### Control Board
- **MCU**: ESP32 (dev kit), Arduino framework, PlatformIO
- **Solenoid pins** (TPS1H200A drivers, 5 channels):
  - S2 (Outhole): GPIO23, 45ms pulse
  - S3 (Inlane): GPIO19, 45ms pulse
  - S4 (Slingshot): GPIO18, 40ms pulse
  - S5 (Bumper): GPIO5, 40ms pulse
  - S6 (Slingshot): GPIO17, 40ms pulse
- **Headbox lamps** (595 shift-register):
  - DATA: GPIO14, CLOCK: GPIO27, LATCH: GPIO26, OE_N: GPIO12
  - Scoring display + attract mode
- **OLED**: GPIO21/22 (I2C main bus)
- **Power rails**:
  - 5V logic
  - 3.3V for ESP32
  - 18V for lamp matrix (separate)
  - 22–26V for solenoids (separate, high-side switch input to TPS1H200A)

#### Matrix Board
- **MCU**: ESP32-C6, ESP-IDF framework
- **Lamp matrix**: 8×4 (28 lamps), 6.3V incandescent
  - Shift-register pins: GPIO15 (DATA), GPIO22 (CLK), GPIO23 (LATCH), GPIO10 (OE_N)
  - Row drivers: VNQ7E100AJTR (quad, active-high outputs)
  - Column inputs: GPIO18–21 (SW_COL_0–3), active-low
- **Switch matrix**: 8×4 (32 switches)
  - Row advance: 5ms cadence, full cycle = 40ms
  - Edge detection + debounce at matrix_interface_runtime.cpp (20ms window)
- **I2C** (to be implemented):
  - Slave at 0x24 on GPIO2 (SDA) / GPIO3 (SCL)
  - **TODO**: Configure in main.cpp
- **OLED**: GPIO6/7 (separate I2C master for status display)
- **Power**: 5V logic, 3.3V for ESP32-C6, 6.3V for lamps (separate rails)

### Switch & Lamp Naming (captain_mapping.h, Locked)
**32 Switches (8×4 matrix)**:
- S20 Outhole, S1 Lane A, S2 Lane B, S3 Lane C, S4 Lane D, S5 Tilt, S6 Outlane Left, S7 Outlane Right
- S10 Inlane Left, S11 Inlane Right, Bumper A, Bumper B, Bumper C, Bumper D, Slingshot L, Slingshot R
- (8 more spare rows reserved)

**40 Lamps (8×5 matrix)**:
- Game Over, Lanes A/B/C/D, Bumper indicators, Bonus display, Player indicators, Ball indicators
- (Remaining slots for future)

---

## 5. Code Archaeology: What Exists & What's Reusable

### High-Confidence Reusable Modules

**Shift-Register Frame Composition** (`matrix main.cpp: sr_compose_frame()`)
```cpp
uint16_t sr_compose_frame(uint8_t rowByte, uint8_t colByte) {
  // Applies active-low inversion if needed
  // Daisy-chain order: U5 (columns) first, U4 (rows) second
  // Returns 16-bit frame ready for sr_shift_frame()
}
```
**Status**: Proven on matrix hardware, should be copied to control-board HAT driver if needed.

**I2C Master Register Read Pattern** (`matrix_interface_runtime.cpp: readRegisterBlock()`)
```cpp
bool readRegisterBlock(uint8_t regStart, uint8_t* pBuf, size_t count) {
  Wire.beginTransmission(0x24);
  Wire.write(regStart);
  Wire.endTransmission(false);
  Wire.requestFrom(0x24, count, true);
  for (size_t i = 0; i < count && Wire.available(); i++) {
    pBuf[i] = Wire.read();
  }
}
```
**Status**: Working, verified against matrix board; use as template for any other I2C master patterns.

**Switch Edge Emission** (`matrix_interface_runtime.cpp: emitSwitchEdges()`)
- Compares previous/current switch bytes
- Calls user callback for each edge transition
- **Pattern**: Proven debounce + edge logic, reuse for any button/switch polling

**Headbox Lamp Attract Pattern** (`headbox_runtime.cpp: attractPattern()`)
- Chases lamps in sequence (24-step pattern defined in captain_mapping.h)
- Ready to adapt for ROM-driven gameplay
- **Status**: Proven, tested

**FreeRTOS Task Structure** (both boards)
- Both control and matrix boards use FreeRTOS with multiple tasks
- Proven pattern: separate tasks for I/O polling, display updates, time-critical solenoid pulses
- Ready to extend for scoring, audio queuing

### Historical Code Available (Not Yet Migrated)

**Home-Edition Behavior** (if needed for reference):
- Audio effects queue system (potentially in home-edition repo)
- OTA two-stage system (control-board infrastructure present)
- Sound effects ROM (if available, can be ported)

---

## 6. Testing & Validation Checklist

### Pre-Integration (Unit-level)
- [ ] Matrix board I2C slave responds to reads at 0x24 (use I2C scanner or control-board probe)
- [ ] Matrix board shift-register lamp updates visible on hardware
- [ ] Control board GPIO solenoid outputs toggle correctly (verify with multimeter or oscilloscope)

### Integration (System-level)
- [ ] Control board serial log shows `matrix detected = true`
- [ ] Control board serial log shows `linkHealthy = true` (within 1s of startup)
- [ ] Control board can write lamp rows; matrix board reflects on shift-register output
- [ ] Control board can read switch state; matrix board reports correct closures
- [ ] Physical switch press → solenoid pulse triggers (audible click + GPIO toggle)

### Audio Validation (When Audio Wired)
- [ ] Solenoid trigger event plays audio effect
- [ ] No switching noise on 26V solenoid rail (oscilloscope check: <100mV ripple at 20MHz bandwidth)

---

## 7. File Structure & Key Locations

```
Captain-Fantastic-Control-and-Hat-Board/
├── firmware/
│   └── control-board/
│       ├── src/
│       │   ├── main.cpp                      ← Game loop, callback stub at onMatrixSwitchEdge()
│       │   ├── input/
│       │   │   └── matrix_interface_runtime.cpp  ← I2C master polling 0x24
│       │   ├── headbox/
│       │   │   └── headbox_runtime.cpp       ← 595 lamp drive
│       │   ├── audio/
│       │   │   └── audio_runtime.cpp         ← Audio infrastructure
│       │   └── ota/
│       ├── include/
│       │   ├── captain_protocol.h            ← Register map (LOCKED)
│       │   ├── captain_mapping.h             ← Switch/lamp names (LOCKED)
│       │   ├── solenoid_gpio_config.h        ← GPIO assignments (LOCKED)
│       │   └── i2c_bus_config.h              ← I2C bus config (0x24 slave addr)
│       └── platformio.ini                    ← Build config (FIXED: removed version pin)

Pinball-Sw-Lamp-Matrix-/
├── firmware/
│   └── src/
│       ├── main.cpp                          ← **TODO: Add I2C slave at 0x24**
│       └── CMakeLists.txt
```

---

## 8. Next Developer Handoff Instructions

### Setup
1. Clone both repositories:
   ```
   C:\cfhe\Pinball-Sw-Lamp-Matrix\firmware\ (no spaces, ESP-IDF requirement)
   Captain-Fantastic-Control-and-Hat-Board/firmware/control-board/
   ```
2. Install dependencies:
   - PlatformIO + ESP32 toolchain (control board)
   - ESP-IDF + ESP32-C6 toolchain (matrix board)
3. Verify CI build passes (GitHub Actions run 26+)

### Immediate Action
**Start with Priority 1**: Implement I2C slave at 0x24 on matrix board
- Follow the register protocol in Section 4
- Use `driver/i2c_slave.h` as reference
- Test with control-board I2C master polling (20ms interval)

### Success Criteria for Each Priority
- **Priority 1**: `linkHealthy = true` in control-board serial log
- **Priority 2**: Solenoid pulse triggered by physical switch press
- **Priority 3**: Score increments, lamps update
- **Priority 4**: Audio plays on solenoid event, no noise on power rail

### Documentation Updates
- [ ] Update PROJECT_STATUS.md when Priority 1 is complete
- [ ] Update FIRMWARE_MIGRATION.md when Priority 2 is complete
- [ ] Create AUDIO_MIGRATION_NOTES.md when starting Priority 4

---

## 9. Known Issues & Gotchas

| Issue | Impact | Workaround | Status |
|-------|--------|-----------|--------|
| platformio.ini pinned ESP-IDF version | CI build failure | Removed platform_packages line | ✅ Fixed |
| Matrix board no I2C slave | Integration blocked, system won't init | Implement Priority 1 | ⏳ Pending |
| Control-board onMatrixSwitchEdge() empty | No solenoid coupling | Implement Priority 2 | ⏳ Pending |
| Audio not wired to game events | Audio doesn't play | Wire Priority 4 | ⏳ Pending |

---

## 10. Contact Points & References

**Original Discovery**: This handoff document was compiled from:
- Full codebase audit of both repos (control-board and matrix-board)
- GPIO assignments verified in solenoid_gpio_config.h and captain_mapping.h
- Register protocol locked in captain_protocol.h and validated across both boards
- I2C master polling verified working in matrix_interface_runtime.cpp

**Previous Commits**:
- **CI fix**: Removed platformio.ini `platform_packages` line (run 26 pending)
- **Last working matrix board state**: Standalone shift-register + GPIO scan operational
- **Last working control board state**: I2C master polling enabled, awaiting I2C slave response

**Architecture Decision Log**:
- Chose HT16K33-style I2C register model for simplicity and proven track record
- Chose TPS1H200A for solenoid drivers (single-channel, simple GPIO interface, safe gate drive)
- Chose ESP32-C6 for matrix board (native I2C slave support, strong PWM capabilities)

---

**Last Updated**: 2026-04-29  
**Next Review**: After Priority 1 completion  
**Owner**: Next Developer (take over from here)

