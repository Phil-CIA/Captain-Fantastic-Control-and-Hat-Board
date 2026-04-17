# System Behavior Contract - Captain Fantastic Matrix Board Firmware

Status: User answers integrated
Last Updated: 2026-04-17
Owner: [You]

This contract is the matrix-board companion to the control-board system contract.

Design boundary (locked):
- Matrix board owns switch scanning, debounce, edge/state reporting, and lamp multiplex timing.
- Control board owns game state, scoring, audio, diagnostics policy, and lamp intent decisions.
- Matrix board should be deterministic and timing-focused; avoid embedding game rules.

## Recommended v1 Direction (analysis only; not yet locked)

This board should stay simple, deterministic, and easy to debug in the first playable build. The safest recommendation is to prefer stable state reporting over rich event semantics, strict validation over permissive behavior, and safe lamp hold/off behavior over clever recovery.

### Suggested defaults to consider while answering
- **Interface:** lean toward a register-map-first or hybrid path, expose a protocol version field, and publish a debounced bitmap plus changed-state mask.
- **Link behavior:** prefer a 250 ms heartbeat target, a ready flag, and holding the last safe lamp state on simple link loss.
- **Scan/debounce:** start around a 5 ms scan interval and 20 ms debounce window, with one closure report until release.
- **Noise handling:** suppress duplicates, trust the last stable debounced state, and count noisy/invalid conditions for diagnostics.
- **Boot policy:** report stuck switches, block ready until the issue is understood or overridden in service mode, and keep the override explicit.
- **Lamp application:** apply valid writes promptly, refresh from retained intent, and use atomic row/block update behavior to avoid visible glitches.
- **Diagnostics:** expose fault/status registers, keep counters for timeouts and invalid writes, and reject malformed commands without disturbing the current safe state.
- **Bench bring-up:** allow direct lamp writes, synthetic switch injection, and a one-shot status dump for early cabinet-free testing.
- **Round-question bias:** fixed version register, shadow-buffer commit for lamp writes, per-switch bitmask, no runtime debounce tuning in v1, binary lamp on/off only for MVP, and self-test available from both control command and local service access.

### Locked-answer consistency note
A few answers were marked as uncertain during review. To keep the contract implementable, the document uses these conservative provisional defaults: expose the ready flag to the control board, count non-fatal noise events, allow matrix lamp-walk primitives, use per-switch bitmask if a change mask is later enabled, and keep lamp output binary on/off only for v1.

---

## I. Interface Contract (Control <-> Matrix)

### DECISION 1: v1 Transport Shape

Question: What transport model should v1 use between control board and matrix board?

Options:
- A) I2C register map (snapshot + status registers)
- B) Framed packet stream
- C) Hybrid (register map now, packet framing later)

Decision:
```
Transport: [x] A) I2C register map  [ ] B) Framed packet  [ ] C) Hybrid path
Protocol version field exposed to control board: [x] Yes  [ ] No
```

### DECISION 2: Switch Data Publication Mode

Question: What switch information should matrix board publish in v1?

Options:
- A) Debounced full bitmap only
- B) Edge events only
- C) Debounced bitmap + changed-state mask

Decision:
```
Publication mode: [x] A) Bitmap  [ ] B) Edge only  [ ] C) Bitmap + change mask
Include release transitions: [ ] Yes  [x] No
```

### DECISION 3: Link Supervision Behavior

Question: What supervision behavior should matrix board implement?

Decision:
```
Heartbeat period target: [ ] 100 ms  [ ] 250 ms  [x] 500 ms
Expose ready flag to control board: [x] Yes  [ ] No
On control link loss, lamp behavior: [x] Hold last state  [ ] All off  [ ] Diagnostic pattern
```

---

## II. Switch Scanning and Debounce Policy

### DECISION 4: Scan and Debounce Defaults

Question: What scan/debounce defaults should matrix board enforce?

Decision:
```
Scan interval: [ ] 2 ms  [ ] 5 ms  [x] 10 ms
Debounce window: [ ] 10 ms  [x] 20 ms  [ ] 30 ms
One report per closure until release: [x] Yes  [ ] No
```

### DECISION 5: Conflict and Duplicate Handling

Question: How should matrix board handle duplicate or noisy transitions?

Decision:
```
Suppress duplicate closure reports while held: [x] Yes  [ ] No
If state is ambiguous/noisy, prefer last debounced stable state: [x] Yes  [ ] No
Raise non-fatal noise counter for diagnostics: [x] Yes  [ ] No
```

### DECISION 6: Boot-Time Switch Health

Question: What should matrix board do with stuck switches at boot?

Decision:
```
Report stuck switch list to control board: [x] Yes  [ ] No
Block ready flag until cleared: [x] Yes  [ ] No
Allow operator override command in service mode: [x] Yes  [ ] No
```

---

## III. Lamp Drive and Refresh Policy

### DECISION 7: Lamp Intent Application

Question: How should matrix board apply control-board lamp intents?

Decision:
```
Apply lamp intents immediately on valid write: [x] Yes  [ ] No
Also refresh from last intent on heartbeat cadence: [x] Yes  [ ] No
Require complete row block write before visible update (atomic swap): [ ] Yes  [x] No
```

### DECISION 8: Safe State Rules

Question: What safe-state rules should matrix board enforce locally?

Decision:
```
On internal matrix fault, force lamps off: [x] Yes  [ ] No
On link timeout, keep last safe lamp state: [x] Yes  [ ] No
Expose fault code register for control-board polling: [x] Yes  [ ] No
```

### DECISION 9: Lamp Test Ownership

Question: Which board owns low-level lamp walk tests?

Decision:
```
Matrix board provides lamp walk primitive commands: [x] Yes  [ ] No
Control board orchestrates test sequences: [x] Yes  [ ] No
Allow standalone matrix self-test mode: [x] Yes  [ ] No
```

---

## IV. Diagnostics and Service Hooks

### DECISION 10: Counters and Telemetry

Question: What counters should matrix board maintain for service diagnostics?

Decision:
```
Count switch scan cycles: [ ] Yes  [x] No
Count debounce suppressions: [x] Yes  [ ] No
Count invalid command frames/register writes: [ ] Yes  [x] No
Count link-timeout events: [x] Yes  [ ] No
```

### DECISION 11: Command Validation Strictness

Question: How strict should matrix board be on malformed writes?

Decision:
```
Reject out-of-range writes and keep prior state: [x] Yes  [ ] No
Set a latched warning flag when invalid write occurs: [x] Yes  [ ] No
Clear warning flag only by explicit control-board command: [x] Yes  [ ] No
```

### DECISION 12: Bench Bring-Up Mode

Question: What bench features should be available before full cabinet integration?

Decision:
```
Allow direct lamp row/bit writes via debug command path: [x] Yes  [ ] No
Allow synthetic switch injection for protocol test: [x] Yes  [ ] No
Expose one-shot status dump command: [ ] Yes  [x] No
```

---

## V. Round 1 Clarification Questions

### Q1: Register Map Versioning
```
Publish protocol version at fixed register address: [x] Yes  [ ] No
Major/minor version split: [ ] Yes  [x] No
```

### Q2: Read/Write Atomicity
```
Require shadow buffer + commit bit for lamp writes: [x] Yes  [ ] No
```

### Q3: Change Mask Width
```
Change mask format: [x] Per-switch bitmask  [ ] Per-row byte mask
```

### Q4: Debounce Tuning Access
```
Allow runtime scan/debounce tuning registers in v1: [ ] Yes  [x] No
```

### Q5: Diagnostic Fault Classes
```
Fault classes exposed: [x] Link  [x] Switch stuck  [x] Lamp driver  [ ] Internal timing
```

---

## VI. Round 2 Clarification Questions

### Q6: Link Loss Transition
```
Link-loss timeout source: [x] Control heartbeat missing  [ ] Matrix watchdog local timer
```

### Q7: Ready State Contract
```
Ready requires all self-checks passed: [x] Yes  [ ] No
Ready can be true during degraded mode with warnings: [x] Yes  [ ] No
```

### Q8: Lamp PWM/Brightness
```
Binary on/off only in v1: [x] Yes  [ ] No
Include brightness levels in v1: [ ] Yes  [x] No
```

### Q9: Error Counter Persistence
```
Persist counters across reboot: [x] Yes  [ ] No
```

### Q10: Matrix Self-Test Trigger
```
Self-test trigger source: [x] Control command  [ ] Local mode pin/switch  [ ] Both
```

---

## VII. Round 3 Clarification Questions

### Q11: Lamp Write Atomicity Scope
Rationale: DECISION 7 says visible updates do not require full-row atomic swap, while Q2 enables shadow buffer + commit. This needs a single v1 rule.
```
Lamp update mode in v1: [ ] Immediate per-byte apply  [ ] Shadow buffer + explicit commit required
If commit is required, max apply latency target: [ ] <= 5 ms  [ ] <= 10 ms  [ ] <= 20 ms
```

### Q12: Ready vs Degraded Definition
Rationale: Q7 allows ready=true with degraded warnings while also requiring self-check pass. Define what is non-fatal.
```
Allowed while ready=true (non-fatal degraded): [ ] Missing optional telemetry  [ ] One or more stuck switches (operator override active)  [ ] Link warning history only
Disallowed while ready=true (fatal): [ ] Active lamp driver fault  [ ] Scan engine timing violation  [ ] Register-map CRC/validation failure
```

### Q13: Counter Persistence Policy
Rationale: Q9 enables reboot persistence; this needs storage and lifecycle policy to prevent wear and unclear reset semantics.
```
Persisted counter write policy: [ ] Every event  [ ] Periodic checkpoint (time-based)  [ ] Periodic checkpoint (delta/event-count based)
Counter reset authority: [ ] Control command only  [ ] Local service action only  [ ] Both
```

### Q14: Release Semantics for Gameplay Logic
Rationale: DECISION 2 disables release transitions, but some game logic may require release detection.
```
Release information source in v1: [ ] Not exposed (control infers from bitmap polling)  [ ] Explicit release bitmask register  [ ] Optional service-only release telemetry
If inferred, minimum control polling period assumption: [ ] <= 10 ms  [ ] <= 20 ms  [ ] <= 50 ms
```

### Q15: Invalid Write Observability
Rationale: DECISION 10 disables invalid-write counters, while DECISION 11 latches warnings. Define minimum observability and clear behavior.
```
Invalid-write diagnostics in v1: [ ] Latched flag only  [ ] Latched flag + last bad address/value register  [ ] Add counter despite DECISION 10
Warning clear precondition: [ ] Clear anytime by command  [ ] Clear only when bus idle and no new faults since last read
```

---

## VIII. Round 4 Clarification Questions

### Q16: Register Map Baseline (v1 addresses)
Rationale: DECISION 1 and Q1 lock a fixed version register, but v1 integration still needs a concrete baseline map so both boards can compile against identical offsets.
```
Publish a frozen v1 register table in this contract: [ ] Yes  [ ] No
If yes, include explicit addresses for: [ ] Protocol version  [ ] Ready/fault flags  [ ] Switch bitmap  [ ] Change mask (if enabled)  [ ] Lamp write window
Addressing style: [ ] Byte offsets from base  [ ] 16-bit absolute addresses
```

### Q17: Heartbeat and Timeout Numeric Contract
Rationale: DECISION 3 chooses a 500 ms heartbeat target and hold-last behavior, but timeout and recovery thresholds remain undefined.
```
Link-loss timeout threshold: [ ] 750 ms  [ ] 1000 ms  [ ] 1500 ms
Consecutive healthy heartbeats required to clear link-loss state: [ ] 1  [ ] 2  [ ] 3
When link recovers, lamp state source: [ ] Keep held state until next control write  [ ] Reapply full retained intent immediately
```

### Q18: Switch Event Ordering and Snapshot Consistency
Rationale: With bitmap-first publication and optional future change mask, we need one deterministic read-order rule so control logic never mixes epochs.
```
Snapshot consistency rule: [ ] Read flags then bitmap latches same epoch  [ ] Read bitmap then flags latches same epoch  [ ] Explicit snapshot-strobe register required
Change-mask clear behavior (if enabled later): [ ] Clear on read  [ ] Clear on next scan tick  [ ] Clear only on explicit ack write
```

### Q19: Stuck-Switch Override Safety Envelope
Rationale: DECISION 6 allows operator override when stuck switches exist, but runtime constraints for that override are not locked.
```
Override scope: [ ] Per-switch mask  [ ] Global override
Override lifetime: [ ] Until power cycle  [ ] Timed expiry  [ ] Until explicit clear
While override active, service indicator requirement: [ ] Status flag only  [ ] Status flag + periodic log/event pulse
```

### Q20: Counter Persistence Storage Policy
Rationale: Q9 enables reboot persistence, and Q13 requests lifecycle policy. We need a concrete storage medium and wear-safe cadence.
```
Persistence medium: [ ] NVS/flash  [ ] External EEPROM/FRAM  [ ] RAM only until shutdown command
Checkpoint cadence target: [ ] Every 30 s  [ ] Every 60 s  [ ] Every 100 counter deltas
Power-fail tolerance requirement: [ ] Best effort  [ ] Last complete checkpoint guaranteed
```

---

## IX. Next Steps

Once the decisions above are locked, implementation sequence for matrix board should be:
1. Register map and version field
2. Scan/debounce engine with stable-state reporting
3. Link supervision and ready/fault flags
4. Lamp intent apply path with safe-state handling
5. Service telemetry counters and status dump
6. Integration test with control board protocol runtime

---

## X. Meta: Document Revision Log

| Date | Status | Changes |
|------|--------|---------|
| 2026-04-17 | Draft | Created initial matrix-board contract with iterative decision/question rounds |
| 2026-04-17 | Guidance added | Added concise recommendation notes for answering the matrix-board decision set |
| 2026-04-17 | Locked user answers | Integrated the v1 I2C register-map choices, debounce policy, safe-state rules, diagnostics, and round-1/round-2 answers |
