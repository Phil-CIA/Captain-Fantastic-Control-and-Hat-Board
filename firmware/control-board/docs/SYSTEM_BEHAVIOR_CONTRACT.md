# System Behavior Contract — Captain Fantastic Control Board Firmware

## System Partition Note

This contract now reflects the current multi-board split:
- the matrix board owns switch scanning, debounce, and lamp-drive timing
- the control board owns game state, scoring, audio, diagnostics, display behavior, and high-level output policy
- the control board should consume debounced events and send high-level lamp intent rather than directly multiplex the lamp or switch matrix

**Status:** 🟡 Initial defaults selected  
**Last Updated:** 2026-04-16  
**Owner:** [You]  
**Reference Sources:** 
- `Captain-Fantastic-home-edition/src/main_firmware.cpp` (lines 105-1143: game state machine, scoring, bonus, diagnostics)
- `Captain-Fantastic-home-edition/docs/MP3_MUSIC_SETUP.md` (audio architecture intent)
- `Captain-Fantastic-home-edition/docs/WIFI_STREAMING_SETUP.md` (hybrid storage/streaming model)
- `Captain-Fantastic-home-edition/docs/DIAGNOSTIC_TEST_MODE.md` (Bally diagnostic specification)

---

## I. GAME STATE MACHINE

**Authoritative Source:** `main_firmware.cpp` lines 1024-1143

### Six States (50ms cycle interval)
1. **ATTRACT** — Idle loop, random lamp chase, attract music, awaiting START button
2. **GAME_START** — Player pressed START, play start fanfare, initialize scoring/bonus/multiplier
3. **BALL_IN_PLAY** — Active gameplay, switches score points, solenoids respond, lamps track features
4. **BONUS_COUNTDOWN** — Ball drained, count down bonus to score, play countdown music
5. **BALL_OVER** — End of bonus countdown, return to ATTRACT
6. **GAME_OVER** — All balls exhausted, play game-over fanfare, flash high-score lamps

### State Transitions
```
ATTRACT --[START button]--> GAME_START
GAME_START --[immediate]--> BALL_IN_PLAY
BALL_IN_PLAY --[drain/tilt]--> BONUS_COUNTDOWN
BONUS_COUNTDOWN --[countdown ends]--> BALL_OVER
BALL_OVER --[immediate OR new ball time]--> BALL_IN_PLAY (repeat) OR GAME_OVER (if no balls left)
GAME_OVER --[display high score]--> ATTRACT
```

---

## II. SCORING & SWITCH MATRIX (22 Switches)

**Authoritative Source:** `main_firmware.cpp` lines 620-754

### Switch Functions & Point Values

| Switch | Point Value | Solenoid Coupling | Feature Logic |
|--------|-------------|-------------------|---------------|
| Bumper 1 | 100 | None | Kickback ready if lit |
| Bumper 2 | 100 | None | Kickback ready if lit |
| Bumper 3 | 100 | None | Kickback ready if lit |
| Lane A | 500 | None | Lane complete tracking → Extra ball at A-B-C-D |
| Lane B | 500 | None | Lane complete tracking |
| Lane C | 500 | None | Lane complete tracking |
| Lane D (Return Left) | 1000 | Multiplier advance | Advance return lane 1X→2X→3X→5X on successive completions |
| Lane E (Return Right) | 1000 | Multiplier advance | Advance return lane multiplier |
| Target 1 | 250 | Solenoid-1 | Target progression 1-2-3 → Double/Triple bonus at 3-complete |
| Target 2 | 250 | Solenoid-2 | Target progression |
| Target 3 | 250 | Solenoid-3 | Target progression |
| Rollover 1 | 300 | None | Bonus pole increment (1K-19K) |
| Rollover 2 | 300 | None | Bonus pole increment |
| Rollover 3 | 300 | None | Bonus pole increment |
| Spinner | 750 | None | Bonus multiplier advance or score |
| Jet Bumper | 50 | Solenoid-4 | Rapid drain risk, eject on hit |
| Ramp 1 (Start) | 1000 | None | Ramp combo tracking |
| Ramp 2 (Mid) | 2000 | None | Ramp combo tracking |
| Ramp 3 (End) | 5000 | Solenoid-5 | Ramp complete → Feature lit or bonus advance |
| Slingshot Left | 10 | Multiplier advance | Advance slingshot multiplier on hit |
| Slingshot Right | 10 | Multiplier advance | Advance slingshot multiplier on hit |
| Tilt | 0 | Tilt solenoid | End ball immediately, disable flipper, drain ball fast |

### Extra Ball Awards
- 100K points → Extra ball light
- 200K points → Extra ball light
- 300K points → Extra ball light

### Bonus Pole (Displayed as 1K-19K)
- Incremented by Rollover switches (each +1K)
- Counted down at end of ball (1K per 50ms tick = ~20s total countdown)
- Multiplied by Return Lane multiplier (1X, 2X, 3X, or 5X)

---

## III. AUDIO ARCHITECTURE

**Authoritative Sources:** 
- `main_firmware.cpp` lines 301-341 (sound effect enums, music track enums)
- `main_firmware.cpp` lines 1300-1435 (audio function definitions)
- `MP3_MUSIC_SETUP.md` (intended storage and policy)

### Sound Effects (13 Types)
1. Bumper hit (700 Hz, 50ms)
2. Slingshot (rising tone 400→900 Hz, 100ms)
3. Target hit (1300 Hz, 80ms)
4. Rollover (bell tone, 200ms)
5. Ball drain (descending 1000→200 Hz, 300ms)
6. Bonus counting (1000 Hz, rapid pulses 100ms each)
7. Feature lit (fanfare 2-note melody)
8. Extra ball (descending fanfare)
9. Startup tone (system boot confirmation)
10. Game start (game-start fanfare, ~2-3s)
11. Game over (game-over fanfare, ~2-3s)
12. Tilt (harsh buzzer, 500ms)
13. Switch open/close debug (low tone debug alert)

### Music Tracks (5 Roles)

| Track Role | Filename | Duration | Loop Policy | Volume |
|-----------|----------|----------|-------------|--------|
| Attract | /attract.mp3 | 3-5 min | Loop indefinitely | 0.3-0.4 (background) |
| Game Start | /game_start.mp3 | 1-3 sec | Play once | 0.8 (prominent) |
| Bonus Countdown | /bonus.mp3 | 20 sec | Play once | 0.6-0.7 |
| Game Over | /game_over.mp3 | 2-3 sec | Play once | 0.8 (prominent) |
| High Score | /hiscore.mp3 | 3-5 sec | Play once | 0.8 (prominent) |

#### Legacy Implementation Status
- ✅ Music functions defined: `requestMusic()`, `playMusic()`, `updateMusicPlayer()`
- ✅ Audio event matrix defined (game state → music trigger mapping)
- ❌ **NOT implemented:** Music functions **never called** from game state machine in current main_firmware.cpp
- ❌ **NOT implemented:** Audio task explicitly disables MP3 playback ("MP3 playback disabled - sound effects only")

---

## IV. STORAGE BUDGET MODEL

**Calculation Basis:** ESP32 app partition ~1.31MB; W25Q128 external flash 16MB

| Component | Size | Notes |
|-----------|------|-------|
| Firmware binary | ~1.12 MB | Current code with all subsystems |
| OTA staging reserve | ~2.0 MB | Conservative; covers max firmware + overhead |
| Usable audio library | ~14 MB | Remaining for MP3 assets on W25Q128 |

### Audio Capacity (W25Q128 14MB)
- **At 128 kbps:** ~14-15 minutes total content
- **At 96 kbps:** ~19-20 minutes total content
- **Recommended:** Attract (3min) + 4 game tracks (30s each) = ~5 min total; leaves 9-15 min for future expansion

### Secondary Storage (SPIFFS Internal Flash)
- ~1.9 MB usable for bootstrap/fallback files
- Use for: Short sound cues, startup test tone, fallback attract track if WiFi fails

---

## V. DIAGNOSTIC SYSTEM (Bally Series II)

**Authoritative Source:** `DIAGNOSTIC_TEST_MODE.md`, `main_firmware.cpp` lines 930-1143

### Five-Step Diagnostic Sequence

#### Step 1: Logic/Program Test
- Display "600d" on 7-segment
- Duration: 3 seconds
- Purpose: Confirm CPU can reach diagnostics code

#### Step 2: Score Display Scan
- Increment display 0 → 999999 by 111 each cycle
- Duration: ~90 seconds (cycles until test button released)
- Purpose: Verify all segment drivers and multiplexing

#### Step 3: Lamp Test
- Alternate between two groups (GROUP 1 and GROUP 2)
- GROUP 1: Bumpers, Targets, Returns, extra-ball lamps
- GROUP 2: Lanes, Spinners, Ramps, feature lamps
- 5 cycles each group, ~10 seconds total
- Purpose: Verify all lamp matrix output drivers

#### Step 4: Solenoid Test
- Fire solenoids sequentially: 0 (kickback) → 1 (target 1) → 2 (target 2) → 3 (target 3) → 4 (ramp eject)
- 1 second pulse each
- Duration: ~5 seconds total
- Purpose: Verify all solenoid driver circuits and coil continuity

#### Step 5: Switch Stuck Detection
- Poll all 22 switches for stuckness (constant press without release)
- Error code format: (Row × 10) + Column
  - Example: Row 2, Col 3 → Error code 23
- Duration: Continuous until test button released
- Purpose: Catch stuck/shorted switch inputs before game starts

---

## VI. STARTING DESIGN DECISIONS — INITIAL DEFAULTS

These are the first decisions to unblock implementation. More decisions will likely be added as bring-up continues, but the defaults below give the project a clean starting direction.

### DECISION 1: Audio Storage Model

**Question:** How should the firmware handle audio file storage and playback?

**Options:**
- **A) Local-Only (W25Q128):** All 5 MP3 tracks stored on external flash; no network dependency
- **B) Hybrid (Local + WiFi Streaming):** Attract mode streams unlimited songs from music_server.py on PC, with local fallback tracks if needed
- **C) Defer Streaming:** Local-only now; WiFi streaming as a future feature after core gameplay stabilizes

**Decision:**
```
[ ] A) Local-Only
[ ] B) Hybrid Local + WiFi Streaming
[x] C) Defer Streaming
```

**Reasoning (starting default):**
```
Bring-up first. Keep playback local and remove WiFi or PC-server dependency until core game-state audio is proven stable.
```

**Implications:**
- Choice A keeps the architecture simplest but requires all audio assets to be loaded up front
- Choice B adds flexibility but also adds network, server, and fallback complexity early
- Choice C gives the fastest path to a stable first playable build

---

### DECISION 2: Music Playback Policy

**Question:** Which game events should trigger music, and which events can interrupt?

**Options:**
- **A) Attract-Mode Only:** Full-song MP3 loops only during ATTRACT; no in-game background tracks
- **B) Event-Triggered Full Immersion:** Start event, active play, bonus, and game over all drive music changes
- **C) Hybrid:** Attract music plus start, bonus, and game-over event tracks, but no continuous background music during ball play

**Decision:**
```
Playback Scope: [ ] A) Attract-Only  [ ] B) Full Immersion  [x] C) Hybrid Events
Interrupt on Tilt: [x] Yes  [ ] No
Interrupt on Drain: [x] Yes  [ ] No
```

**Reasoning (starting default):**
```
Use attract and event fanfares without continuous in-play background music. This preserves gameplay clarity and keeps the first pass easier to validate.
```

**Implications:**
- A is the simplest but can make active gameplay feel too quiet
- B is richest but creates the most event and state-management complexity
- C is the best first balance between player feedback and implementation effort

---

### DECISION 3: Canonical Game Logic Source

**Question:** Which legacy code repository should be the authoritative source for scoring, state machine behavior, and bonus logic during migration?

**Current Status:**
- ✅ Authoritative: Captain-Fantastic-home-edition/src/main_firmware.cpp
- 🟡 Reference Only: legacy control-board code for GPIO and hardware patterns

**Decision:**
```
Use home-edition main_firmware.cpp as authoritative source: [x] Agree  [ ] Discuss Further

If discussing further, please note concerns:
None at this time.
```

**Reasoning (starting default):**
```
This preserves known-good game behavior while allowing the new control board work to focus on hardware adaptation instead of re-inventing gameplay logic.
```

---

### DECISION 4: Diagnostic Parity Timeline

**Question:** Should the firmware implement the full Bally Series II diagnostic sequence immediately, or phase it after core gameplay and audio?

**Options:**
- **A) Now (Full Parity):** Implement all 5 diagnostic steps immediately
- **B) Phased:** Core diagnostics now, full sequence after the core game loop stabilizes
- **C) Minimal:** Only the most basic checks now; the rest after v1 is stable

**Decision:**
```
Timeline: [ ] A) Now (Full Parity)  [x] B) Phased (2 weeks)  [ ] C) Minimal v1.0
```

**Reasoning (starting default):**
```
Bring up the core game loop and audio behavior first, while still keeping early diagnostic coverage for logic and display validation.
```

---

## VII. NEXT DECISION SET — TIMING AND SAFETY DEFAULTS

These are the next starting defaults to support reliable bench bring-up.

### DECISION 5: Matrix-Board Switch Event Contract

**Question:** What should the matrix board report to the control board for switch activity during the first playable firmware?

**Options:**
- **A) Conservative:** 10 ms scan with 30 ms debounce before reporting events
- **B) Balanced:** 5 ms scan with 20 ms debounce and one event per closure
- **C) Fast:** 2 ms scan with 10 to 15 ms debounce and faster event delivery

**Decision:**
```
Matrix-board scan interval: [ ] A) 10 ms  [x] B) 5 ms  [ ] C) 2 ms
Matrix-board debounce window: [ ] 30 ms  [x] 20 ms  [ ] 10 to 15 ms
Reported edge behavior: [x] One score event per closure until release
Control-board role: [x] Consume debounced events only
```

**Reasoning (starting default):**
```
The responsiveness target stays the same, but the ownership is now on the matrix board. The control board should receive clean switch events rather than raw matrix timing.
```

**Implications:**
- 10 ms and up is very safe, but can make fast switch activity feel soft
- 2 ms is responsive, but may surface more bounce noise during early bring-up
- 5 ms with one event per closure gives strong responsiveness and predictable scoring while keeping the interface clean between boards

---

### DECISION 6: Solenoid Safety and Pulse Policy

**Question:** What safety limits should be applied to gameplay solenoids during early bring-up?

**Options:**
- **A) Strict protection:** 25 to 30 ms default pulse cap, long cooldown, one coil at a time
- **B) Balanced protection:** 35 ms default pulse cap, short retrigger lockout, tune per coil later
- **C) Permissive bring-up:** looser software limits for faster experimentation

**Decision:**
```
Pulse cap default: [ ] A) 25 to 30 ms  [x] B) 35 ms  [ ] C) Loose and tune later
Retrigger lockout: [ ] 50 ms  [x] 125 ms  [ ] Disabled
Disable on tilt or fault: [x] Yes  [ ] No
```

**Reasoning (starting default):**
```
Protect coils first. Conservative software caps reduce the risk of overheating or driver abuse while still allowing realistic bench testing.
```

**Safety rules:**
- A solenoid may not re-fire while it is already active
- A hard timeout always wins over gameplay requests
- Tilt or fault states immediately suppress gameplay solenoid firing
- Per-coil tuning can be added later, but the global safety cap remains in place

---

## VIII. NEXT DECISION SET — PERSISTENCE AND BOARD PROTOCOL DEFAULTS

These defaults define what the control board remembers and how it exchanges intent and status with the matrix board.

### DECISION 7: Persistence and Save Policy

**Question:** What game and service data should persist across power cycles?

**Options:**
- **A) Minimal:** only the most basic service settings persist
- **B) Balanced:** high scores, audits, and service settings persist locally
- **C) Full logging:** detailed per-game history and event logs persist

**Decision:**
```
Data scope: [ ] A) Minimal  [x] B) Balanced  [ ] C) Full logging
Storage owner: [x] Control board local nonvolatile storage
Write timing: [ ] Every event  [x] End-of-ball and settings-change checkpoints  [ ] Manual save only
Factory reset path: [x] Yes  [ ] No
```

**Reasoning (starting default):**
```
Keep the machine useful and serviceable without creating unnecessary flash wear or implementation complexity in the first pass.
```

**Default persisted items:**
- high score table
- service and bookkeeping counters
- audio and gameplay option flags
- protocol or configuration version marker for upgrades

---

### DECISION 8: Matrix-Board Command Protocol and Lamp Intent Model

**Question:** How should the control board and matrix board exchange gameplay information?

**Options:**
- **A) Raw mirror:** matrix board streams raw row and column state and the control board interprets everything
- **B) Event and intent split:** matrix board sends debounced switch and health events; control board sends lamp intent and mode commands
- **C) Autonomous matrix:** matrix board handles significant local game logic itself

**Decision:**
```
Interface model: [ ] A) Raw mirror  [x] B) Event and intent split  [ ] C) Autonomous matrix
Heartbeat or ready status: [ ] Optional  [x] Required
Loss-of-link behavior: [x] Fail safe to non-destructive state
Protocol version field: [x] Yes  [ ] No
```

**Reasoning (starting default):**
```
The matrix board should handle the timing-sensitive work, while the control board remains the game-logic authority. That keeps responsibilities clear and easier to debug.
```

**Default message classes:**
- matrix board to control board: switch closed or released events, stuck-switch or fault reports, heartbeat or ready status, diagnostics status
- control board to matrix board: lamp intents, mode changes, diagnostics requests, reset or inhibit commands

**Lamp contract default:**
- the control board expresses feature or lamp intent
- the matrix board owns the actual multiplex timing and low-level lamp-drive behavior

---

## IX. REMAINING DECISIONS QUEUE

These are still likely next after the protocol defaults are in place:

1. Display or headbox integration scope for v1
2. Audio asset format, naming, and fallback behavior
3. Service-menu depth and bookkeeping detail level

---

## X. NEXT STEPS

With the starting defaults above, code can proceed in this sequence:

1. **Audio Module** → integrate game-state-to-music wiring from the state machine
2. **Scoring Module** → port the 22-switch scoring matrix from the proven legacy source
3. **Matrix Interface Module** → apply the matrix-board switch event contract
4. **Driver Safety Module** → apply solenoid pulse limits and lockout rules
5. **Persistence Module** → store high scores, counters, and service settings with safe write timing
6. **Board Protocol Module** → implement matrix-board event and lamp-intent messaging
7. **Bonus Module** → restore countdown logic and multiplier behavior
8. **Diagnostics Module** → implement the phased diagnostic path
9. **Integration Test** → verify state transitions, lamp updates, audio playback, and solenoid firing

---

## VIII. ROUND 2 CLARIFICATION QUESTIONS (Post-Decision Details)

These are the next implementation-locking questions after Decisions 1-4.

### Q1: Ball Count and End-of-Game Rules

**Question:** How many balls per game and how do extra balls consume?

**Choices:**
- Balls per game: [ ] 3  [ ] 5
- Extra ball consumes and still decrements ball counter: [ ] Yes  [ ] No
- Max extra balls that can be queued: [ ] 1  [ ] 2  [ ] Unlimited

### Q2: Tilt Penalty Policy

**Question:** What exactly happens on TILT during BALL_IN_PLAY?

**Choices:**
- Forfeit current bonus countdown: [ ] Yes  [ ] No
- Immediately mute all music/effects: [ ] Yes  [ ] No
- Disable all solenoids except safe-drain path: [ ] Yes  [ ] No

### Q3: Bonus Countdown Timing and Audio

**Question:** Confirm bonus payout pacing and sound cadence.

**Choices:**
- Bonus step interval: [ ] 50ms  [ ] 75ms  [ ] 100ms
- Sound pulse on every bonus step: [ ] Yes  [ ] No
- Accelerate countdown when remaining bonus <= 5: [ ] Yes  [ ] No

### Q4: Switch Input Policy

**Question:** What debounce/acceptance rules should runtime use?

**Choices:**
- Debounce window: [ ] 20ms  [ ] 30ms  [ ] 50ms
- Simultaneous hits in same scan frame score all: [ ] Yes  [ ] No
- Re-trigger lockout per switch after hit: [ ] 0ms  [ ] 50ms  [ ] 100ms

### Q5: Solenoid Safety Constraints

**Question:** Final safety limits for coil driving?

**Choices:**
- Max single pulse width: [ ] 30ms  [ ] 40ms  [ ] 50ms
- Minimum cooldown per coil between fires: [ ] 50ms  [ ] 100ms  [ ] 150ms
- Global max concurrent coils: [ ] 1  [ ] 2

### Q6: Attract-to-Game Audio Transition

**Question:** How should attract music behave when START is pressed?

**Choices:**
- Stop attract immediately, then play start fanfare: [ ] Yes  [ ] No
- Fade attract over N ms before start fanfare: [ ] 0  [ ] 250  [ ] 500
- Resume attract only when returning to ATTRACT state: [ ] Yes  [ ] No

### Q7: Missing Audio File Behavior

**Question:** What is fallback behavior when a requested track is missing?

**Choices:**
- Fallback to startup tone family for that event: [ ] Yes  [ ] No
- Log warning on serial/OLED each miss: [ ] Yes  [ ] No
- Suppress repeat warnings after first miss per track: [ ] Yes  [ ] No

### Q8: Diagnostic Mode Entry/Exit Contract

**Question:** How do we enter and leave diagnostics on production hardware?

**Choices:**
- Entry method: [ ] Dedicated test button hold  [ ] Serial command  [ ] Both
- Hold duration to enter (button): [ ] 1s  [ ] 2s  [ ] 3s
- Exit method: [ ] Button release  [ ] Timeout  [ ] Explicit command

### Q9: Score Persistence and Reset Rules

**Question:** What values persist across reboot/power cycle?

**Choices:**
- Persist high score in NVS/flash: [ ] Yes  [ ] No
- Persist operator settings (volume, diagnostics prefs): [ ] Yes  [ ] No
- Factory reset command clears persisted values: [ ] Yes  [ ] No

### Q10: MVP Scope Lock for First Implementation Pass

**Question:** Which feature set defines v0.1 done?

**Choices:**
- [ ] State machine + scoring + no MP3 (tone-only)
- [ ] State machine + scoring + event MP3 (no streaming)
- [ ] State machine + scoring + event MP3 + full diagnostics

---

## Meta: Document Revision Log

| Date | Status | Changes |
|------|--------|---------|
| 2026-04-16 | 🟡 Draft | Created from archaeology phase; awaiting user decisions on the first contract pass |
| 2026-04-16 | 🟡 Initial defaults | Selected the first four starting decisions and added a queue for follow-on decisions |
| 2026-04-16 | 🟡 Timing and safety defaults | Added switch debounce, matrix timing, and solenoid safety starting defaults |
| 2026-04-16 | 🟡 Persistence and protocol defaults | Added local persistence defaults and the matrix-to-control command contract |
| 2026-04-16 | 🟡 Draft+ | Added Round 2 clarification questions for implementation details |
| [NEXT] | 🟢 Locked | [Timestamp when decisions are committed] |
