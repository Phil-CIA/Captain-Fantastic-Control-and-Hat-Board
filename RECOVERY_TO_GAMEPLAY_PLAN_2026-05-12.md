# Recovery-to-Gameplay Integration Plan — 2026-05-12

**Status**: Ready for immediate execution (May 13–14).  
**Purpose**: Advance from hardware validation baseline (matrix timing locked, I2C link healthy, S3–S6 solenoid working) to playable behavior while managing parallel hardware work (S2 relay installation) and switch validation gates.

---

## What We Know (As of May 12 End-of-Day)

✅ **Matrix Timing Baseline Locked**
- Deterministic row scheduler: Blank=50µs, Settle=50µs, Hold post-settle
- One-row-active invariant maintained
- Acceptance test passed (no timing violations detected over 60s baseline)
- Checkpoint commit: `ec90aa2` (home-edition) with timing contract doc

✅ **I2C Transport Healthy**
- Matrix slave at 0x24 stable under continuous polling
- Control master at 100 kHz polls every 15ms without checksum/rejection alarm noise
- Packet counters (`rx_pkts`, `wr_ok`, `rd_ok`) increasing monotonically
- Burst filter active: 4-rising-edge-per-poll threshold suppresses scan noise

✅ **Solenoid S3–S6 Verified**
- TPS1H200A driver path working for kitchen drop (S3), orbit (S4), ramp (S5), scoop (S6)
- Pulse widths {45, 45, 45, 40} ms confirmed in hardware testing
- Force/behavior observed as expected on bench

⚠️ **Solenoid S2 (Outhole) in Workaround**
- TPS1H200A current-limit insufficient even at 0Ω (coil/hardware constraint)
- Relay solution implemented: GPIO23 → automotive 24V relay, full 24V fed to outhole solenoid
- **Relay installation NOT YET COMPLETE** (happening May 13 in parallel)
- Until relay installed: **S2 protected by manual testing limits** (see S2 Coil Protection Policy below)

✅ **Matrix LED Heartbeat Implemented**
- WS2812 RGB status LED on GPIO8 via native RMT
- Green = healthy, Red = I2C fault
- 500ms toggle cadence

✅ **Control Board Heartbeat GPIO Conflict Fixed**
- Moved from GPIO2 (hardware conflict with headbox 74HC595 DATA line) to GPIO16
- No impact on matrix communication

---

## What We Need To Know (Blocking Criteria for Gameplay)

### 1. Switch Detection Quality & Dominance
**Criterion**: One dominant bit per switch, no suppression of real presses by filtering.

- **Current State**: Multiple bits rising per press window; unclear which is true signal vs. noise
- **Method**: A/B capture on both machines
  - A = known-good checkpoint (`aca9ac0`, matrix milestone from May 5)
  - B = current code (May 12 head)
  - Identical hardware wiring, same test sequence
  - 30s quiet baseline + 15–20 deliberate presses per switch on 3 target switches
- **Expected Result**: A shows 1 dominant bit per press, B either matches or shows clear regression pattern
- **Acceptance Gate**: Must demonstrate either current code passes OR clear root cause (burst threshold too strict, debounce too conservative, mapping mode off)

### 2. Tilt & Start Direct-Input Paths
**Criterion**: Both inputs debounce cleanly, no cross-talk with matrix polling.

- **Current State**: GPIO-based debounce at 5ms polling, 3-tick threshold
- **Method**: Manual press test, verify no spurious edges during matrix scan
- **Acceptance Gate**: 20 deliberate presses per input, zero missed detections, zero false positives

### 3. Lamp Delivery to Headbox
**Criterion**: 74HC595 lamp frame writes succeed without corruption.

- **Current State**: Lamp frame compose + XOR checksum in control code
- **Method**: Capture lamp register state via I2C read (0x00–0x07) after write, verify bit pattern
- **Acceptance Gate**: 10 intentional lamp updates, all match commanded state ± latency

---

## S2 Coil Protection Policy (During Relay Installation & Limited Testing)

### When To Use
- **Before relay installed**: Any manual testing of S2 solenoid must follow these limits
- **During relay installation**: Protects hardware from repeated activation thermal stress
- **After relay installed**: Revert to normal operation; protection policy suspended

### Limits
| Parameter | Value | Rationale |
|-----------|-------|-----------|
| **Max activations per group** | 2–3 tries | Coil thermal time constant ~30–60s; risk of overtemp above 3 |
| **Cool-down between attempts** | 10–15s | Minimum for coil to dissipate heat before next try |
| **Cool-down between groups** | 60s | Reset to baseline before next validation sequence |
| **Stop condition** | If coil feels warm OR stalls mid-pulse | Immediate abort; let cool 5–10 min before resume |
| **Operator observation** | Listen/feel for coil hum; watch for current stall | Audible signature change = risk signal |

### Enforcement
1. **Operator logs attempt count** per group (manual tally on bench sheet)
2. **GPIO access disabled** after group-max reached; requires manual override/code change to resume
3. **Automatic abort** if test firmware detects current abnormality (implementation TBD after relay install)

### Example Session
```
Group 1: Attempts 1, 2, 3 → cool 60s
Group 2: Attempts 1, 2, 3 → cool 60s
Group 3: Attempts 1 → coil warm detected → STOP
Wait 10 min, inspect coil, proceed only if cool
```

---

## Execution Plan (May 13–14)

### Parallel Work Streams (Can Run Simultaneously)

#### Stream A: Hardware (S2 Relay Installation) — ~2–4 hours
1. Install automotive relay on S2 driver bench
2. Wire GPIO23 → relay control input
3. Wire 24V supply through relay NO contact → outhole solenoid
4. Verify relay click/engagement during test pulse (GPIO23 high)
5. Test solenoid force recovery: full 24V, no current-limit bottleneck
6. If successful: commit "S2 relay workaround complete, force verified"
7. Resume normal operation; protection policy no longer in effect

#### Stream B: Software (Switch Validation Gate) — ~4–6 hours
1. **Start**: Fetch latest on all three repos (should be clean as of this morning)
2. **Phase 1 – A/B Capture Setup** (1–1.5 hours)
   - Check out `aca9ac0` (known-good) to separate branch/tag in matrix repo
   - Build & flash known-good binary to matrix board (or USB-connected bench board)
   - Build & flash current code (`ec90aa2`) to control board
3. **Phase 2 – Known-Good Baseline** (1–2 hours)
   - Boot matrix with `aca9ac0` firmware
   - Run 30s quiet baseline, capture switch register reads
   - Press target switch (e.g., Easy, Tilt, Start) 15–20 times, log per-press register state
   - Repeat on 2–3 additional switches
   - Save capture as `SWITCH_CAPTURE_A_2026-05-13_knowngood.txt`
4. **Phase 3 – Current Code Capture** (1–2 hours)
   - Flash current matrix firmware (`ec90aa2`)
   - Repeat Phase 2 sequence identically
   - Save as `SWITCH_CAPTURE_B_2026-05-13_current.txt`
5. **Phase 4 – Analysis & Gating Decision** (30–60 min)
   - Compare bit dominance, burst-drop counts, debounce stability
   - If A clean & B fails: Apply feature gates (mapping mode force-on, burst threshold relaxed, debounce reduced)
   - Re-test with gates applied
   - Document gate settings in feature gate control table
   - If both pass or regression confirmed & recoverable: Accept gate and proceed to gameplay coupling

### Gameplay Integration (Begins After Switch Gate Passes)
1. **Mode Scaffolding** (2–3 hours)
   - Define mode enum: Boot → Test → Attract → Gameplay → Error
   - Wire mode transitions (currently missing)
   - Boot sets mode=Test; needs manual mode-select logic
2. **Solenoid Coupling** (2–3 hours)
   - Wire switch edge callback → solenoid pulse lookup
   - Example: Start button → S6 (scoop) + advance to Attract
   - Example: Tilt → S2 (outhole) drain + Gameplay abort
3. **Score & Lamp State** (3–4 hours)
   - Implement basic score increment per action (e.g., +10 per switch)
   - Wire lamp updates to score display (7-segment or dot matrix)
   - Verify lamp frames deliver to headbox without corruption

### Definition of "Done" for This Milestone
✅ Switch validation gate passed (dominance confirmed, no filtering loss)  
✅ Tilt & Start inputs working without cross-talk  
✅ S2 relay installed (if parallel work completes) OR S2 protection policy enforced (if pending)  
✅ Mode scaffolding locked  
✅ One complete switch→solenoid→lamp flow works end-to-end (e.g., Start press → scoop fire → attract mode lamp updates)  
✅ All handoff docs updated with gate results and next blockers  
✅ Repos committed and pushed clean

---

## Blocking Criteria (Do NOT Proceed Without)

### Hard Stop: Switch Recovery Gate
- **Gate**: Each tested switch must yield one dominant bit per press window
- **Measurement**: A/B captures show no regression OR regression traced to recoverable feature gate
- **Enforcement**: Gameplay callback wiring blocked until gate signed off

### Hard Stop: Solenoid S2 Coil Protection
- **Gate**: Manual testing limits enforced OR relay installed and verified working
- **Measurement**: No coil thermal failures during limited testing window
- **Enforcement**: S2 cannot pulse without explicit operator override if limits violated

### Soft Gate (Non-Blocking But Document)
- Mode behavior wiring deferred: mode-select logic not yet implemented
  - Will be added incrementally after switch validation passes
  - Test/Service mode will not auto-transition until implementation complete

---

## Risks & Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Switch A/B captures inconclusive | 2–4 day delay before gameplay coding | Run captures on both boards; if one fails, priority is hardware/wiring isolation, not code tuning |
| S2 relay install delays | Coil protection limits must persist longer | Parallel stream: if relay stalls, continue switch validation; S2 testing deferred to next session |
| Tilt input crosstalk with matrix polling | Tilt abort loses reliability during gameplay | Validate Tilt debounce on bench before committing to full mode integration |
| Lamp 74HC595 writes corrupt | Attract/Gameplay lamp updates fail | Test lamp delivery gate before score display loop implementation |

---

## Repo Commit & Push Checklist

- [ ] Create this document: `RECOVERY_TO_GAMEPLAY_PLAN_2026-05-12.md`
- [ ] Append reference section to `MORNING_START_HANDOFF_2026-05-08.md`
- [ ] Commit both to Captain-Fantastic-Control-and-Hat-Board
- [ ] Verify no outstanding changes in other two repos (should be clean)
- [ ] Push to GitHub (all three repos)
- [ ] Verify GitHub workflows / CI runs (if any) do not block

---

## Next Session Continuation (After Completion)

1. Review captured switch data from A/B analysis
2. Evaluate feature gate necessity (if any)
3. Publish gate-pass or gate-fail decision as new handoff (e.g., `SWITCH_GATE_RESULT_2026-05-13.md`)
4. If passed: Begin mode scaffolding in control code
5. If failed: Debug feature gates and re-test; do not unblock gameplay coupling until gate passes

---

## Historical Context

- **May 1–3**: Timing archaeology, CI version fix (espressif32 7.0.0 regression)
- **May 5**: Known-good matrix checkpoint (`aca9ac0`) created at OLED debug milestone
- **May 8**: Solenoid bring-up and S2 relay workaround planning
- **May 10–12**: Protocol hardening, I2C link health validation, timing contract locked
- **May 12 EOD**: Recovery-to-gameplay plan verbal agreement; S2 protection policy defined; parallel work approved
- **May 13**: (This session) Formalize plan into doc, commit, and execute

---

**End of Recovery-to-Gameplay Integration Plan**
