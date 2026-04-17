# System Behavior Contract - Captain Fantastic Matrix Board Firmware

Status: Under Definition
Last Updated: 2026-04-17
Owner: [You]

This contract is the matrix-board companion to the control-board system contract.

Design boundary (locked):
- Matrix board owns switch scanning, debounce, edge/state reporting, and lamp multiplex timing.
- Control board owns game state, scoring, audio, diagnostics policy, and lamp intent decisions.
- Matrix board should be deterministic and timing-focused; avoid embedding game rules.

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
Transport: [ ] A) I2C register map  [ ] B) Framed packet  [ ] C) Hybrid path
Protocol version field exposed to control board: [ ] Yes  [ ] No
```

### DECISION 2: Switch Data Publication Mode

Question: What switch information should matrix board publish in v1?

Options:
- A) Debounced full bitmap only
- B) Edge events only
- C) Debounced bitmap + changed-state mask

Decision:
```
Publication mode: [ ] A) Bitmap  [ ] B) Edge only  [ ] C) Bitmap + change mask
Include release transitions: [ ] Yes  [ ] No
```

### DECISION 3: Link Supervision Behavior

Question: What supervision behavior should matrix board implement?

Decision:
```
Heartbeat period target: [ ] 100 ms  [ ] 250 ms  [ ] 500 ms
Expose ready flag to control board: [ ] Yes  [ ] No
On control link loss, lamp behavior: [ ] Hold last state  [ ] All off  [ ] Diagnostic pattern
```

---

## II. Switch Scanning and Debounce Policy

### DECISION 4: Scan and Debounce Defaults

Question: What scan/debounce defaults should matrix board enforce?

Decision:
```
Scan interval: [ ] 2 ms  [ ] 5 ms  [ ] 10 ms
Debounce window: [ ] 10 ms  [ ] 20 ms  [ ] 30 ms
One report per closure until release: [ ] Yes  [ ] No
```

### DECISION 5: Conflict and Duplicate Handling

Question: How should matrix board handle duplicate or noisy transitions?

Decision:
```
Suppress duplicate closure reports while held: [ ] Yes  [ ] No
If state is ambiguous/noisy, prefer last debounced stable state: [ ] Yes  [ ] No
Raise non-fatal noise counter for diagnostics: [ ] Yes  [ ] No
```

### DECISION 6: Boot-Time Switch Health

Question: What should matrix board do with stuck switches at boot?

Decision:
```
Report stuck switch list to control board: [ ] Yes  [ ] No
Block ready flag until cleared: [ ] Yes  [ ] No
Allow operator override command in service mode: [ ] Yes  [ ] No
```

---

## III. Lamp Drive and Refresh Policy

### DECISION 7: Lamp Intent Application

Question: How should matrix board apply control-board lamp intents?

Decision:
```
Apply lamp intents immediately on valid write: [ ] Yes  [ ] No
Also refresh from last intent on heartbeat cadence: [ ] Yes  [ ] No
Require complete row block write before visible update (atomic swap): [ ] Yes  [ ] No
```

### DECISION 8: Safe State Rules

Question: What safe-state rules should matrix board enforce locally?

Decision:
```
On internal matrix fault, force lamps off: [ ] Yes  [ ] No
On link timeout, keep last safe lamp state: [ ] Yes  [ ] No
Expose fault code register for control-board polling: [ ] Yes  [ ] No
```

### DECISION 9: Lamp Test Ownership

Question: Which board owns low-level lamp walk tests?

Decision:
```
Matrix board provides lamp walk primitive commands: [ ] Yes  [ ] No
Control board orchestrates test sequences: [ ] Yes  [ ] No
Allow standalone matrix self-test mode: [ ] Yes  [ ] No
```

---

## IV. Diagnostics and Service Hooks

### DECISION 10: Counters and Telemetry

Question: What counters should matrix board maintain for service diagnostics?

Decision:
```
Count switch scan cycles: [ ] Yes  [ ] No
Count debounce suppressions: [ ] Yes  [ ] No
Count invalid command frames/register writes: [ ] Yes  [ ] No
Count link-timeout events: [ ] Yes  [ ] No
```

### DECISION 11: Command Validation Strictness

Question: How strict should matrix board be on malformed writes?

Decision:
```
Reject out-of-range writes and keep prior state: [ ] Yes  [ ] No
Set a latched warning flag when invalid write occurs: [ ] Yes  [ ] No
Clear warning flag only by explicit control-board command: [ ] Yes  [ ] No
```

### DECISION 12: Bench Bring-Up Mode

Question: What bench features should be available before full cabinet integration?

Decision:
```
Allow direct lamp row/bit writes via debug command path: [ ] Yes  [ ] No
Allow synthetic switch injection for protocol test: [ ] Yes  [ ] No
Expose one-shot status dump command: [ ] Yes  [ ] No
```

---

## V. Round 1 Clarification Questions

### Q1: Register Map Versioning
```
Publish protocol version at fixed register address: [ ] Yes  [ ] No
Major/minor version split: [ ] Yes  [ ] No
```

### Q2: Read/Write Atomicity
```
Require shadow buffer + commit bit for lamp writes: [ ] Yes  [ ] No
```

### Q3: Change Mask Width
```
Change mask format: [ ] Per-switch bitmask  [ ] Per-row byte mask
```

### Q4: Debounce Tuning Access
```
Allow runtime scan/debounce tuning registers in v1: [ ] Yes  [ ] No
```

### Q5: Diagnostic Fault Classes
```
Fault classes exposed: [ ] Link  [ ] Switch stuck  [ ] Lamp driver  [ ] Internal timing
```

---

## VI. Round 2 Clarification Questions

### Q6: Link Loss Transition
```
Link-loss timeout source: [ ] Control heartbeat missing  [ ] Matrix watchdog local timer
```

### Q7: Ready State Contract
```
Ready requires all self-checks passed: [ ] Yes  [ ] No
Ready can be true during degraded mode with warnings: [ ] Yes  [ ] No
```

### Q8: Lamp PWM/Brightness
```
Binary on/off only in v1: [ ] Yes  [ ] No
Include brightness levels in v1: [ ] Yes  [ ] No
```

### Q9: Error Counter Persistence
```
Persist counters across reboot: [ ] Yes  [ ] No
```

### Q10: Matrix Self-Test Trigger
```
Self-test trigger source: [ ] Control command  [ ] Local mode pin/switch  [ ] Both
```

---

## VII. Next Steps

Once the decisions above are locked, implementation sequence for matrix board should be:
1. Register map and version field
2. Scan/debounce engine with stable-state reporting
3. Link supervision and ready/fault flags
4. Lamp intent apply path with safe-state handling
5. Service telemetry counters and status dump
6. Integration test with control board protocol runtime

---

## VIII. Meta: Document Revision Log

| Date | Status | Changes |
|------|--------|---------|
| 2026-04-17 | Draft | Created initial matrix-board contract with iterative decision/question rounds |
