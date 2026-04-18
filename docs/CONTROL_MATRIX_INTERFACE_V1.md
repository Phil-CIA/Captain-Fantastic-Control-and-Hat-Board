# Control ↔ Matrix Board Interface Contract (v1)

**Status:** Locked as the intended stable control-to-matrix behavior contract  
**Last Updated:** 2026-04-18  
**Purpose:** Define how the control board should treat the matrix board for MVP integration.

This document intentionally keeps the **HT16K33-style operating concept** for the split-board system. The point is not to let the control board micromanage matrix timing; the point is to give the control board a simple, durable peripheral model while the matrix-board firmware stays free to change internally whenever needed.

**Important framing:**
- this is the **preferred inter-board behavior contract**
- the matrix board may still use local GPIO, shift registers, and bring-up-only firmware details under the hood
- those local implementation details do **not** require abandoning the simple external HT16K33-style model

---

## 1. Board Ownership Boundary

### Control board owns
- game state machine
- scoring and bonus logic
- audio behavior
- diagnostics policy
- persistence and operator settings
- lamp intent decisions
- system-level fault handling policy

### Matrix board owns
- switch matrix scanning
- switch debounce and stable-state publication
- lamp multiplex timing and low-level drive execution
- boot-safe output behavior
- local implementation details needed to satisfy the external contract

### Explicit non-goals for v1
- the control board does **not** tune scan rate, debounce policy, or row/column service sequencing during normal play
- the matrix board does **not** own gameplay rules, scoring rules, or feature progression

---

## 2. Preferred v1 Contract Model

- **Bus:** I2C
- **Role split:** control board = master, matrix board = slave/peripheral
- **Preferred slave address:** **0x24**
- **Wire model:** HT16K33-style command plus register-pointer interface
- **Master pattern:** write simple global commands directly, write a register pointer then payload for lamp image updates, and read stable switch/diagnostic snapshots by pointer + repeated-start read

### Implementation-freedom note
The matrix firmware is free to implement this behavior using whatever internal method is most practical, including local GPIO control, shift-register expansion, local scan loops, or later firmware refactors. That internal method is **not** the control-board contract.

There is **no required framed packet protocol** in current v1.

---

## 3. Canonical v1 Data Structures

### 3.1 Lamp image written by control board

```c
uint8_t lampRows[8];
```

- register window: **0x00..0x07**
- one byte per row
- lower **4 bits** are active lamp columns for that row
- upper 4 bits are reserved and must be written as 0
- the matrix board retains the last valid lamp image until it is overwritten or outputs are disabled

### 3.2 Switch snapshot read by control board

```c
uint8_t switchBytes[4];
```

- register window: **0x40..0x43**
- packed representation of the **8 x 4** switch matrix
- each bit represents the latest debounced stable state for one switch position
- **bit value 1 = switch closed / active**
- control logic should compare the newest snapshot against the previous snapshot to detect new closures

### 3.3 Diagnostic snapshot

```c
typedef struct {
  uint8_t statusFlags;
  uint8_t pulseLevel;
  uint8_t lampSample;
  uint8_t switchSample;
} MatrixDiagV1;
```

- register window: **0xF0..0xF3**
- current agreed meanings:
  - **0xF0** = status flags
    - bit 0 = system enabled
    - bit 1 = output enabled
    - bit 2 = test override active
    - bit 3 = auto-walk active
  - **0xF1** = current pulse-width level, 0..15
  - **0xF2** = live echo/sample of lamp row 0
  - **0xF3** = live echo/sample of switch byte 0

### 3.4 Matrix-local runtime state

The matrix board keeps these as internal state variables:
- system enabled or disabled
- output enabled or disabled
- lamp pulse-width level, 0..15
- latest retained lamp-row image
- latest debounced switch snapshot

---

## 4. Minimal v1 Instruction Set

### 4.1 Global command bytes

| Command | Meaning | Expected effect |
|--------:|---------|-----------------|
| **0x20** | System disable | stop active matrix service behavior and force safe inactive state |
| **0x21** | System enable | enable normal matrix scanning and runtime operation |
| **0x80** | Output disable | disable lamp outputs while keeping configuration state |
| **0x81** | Output enable | allow lamp output using the retained lamp image |
| **0xE0..0xEF** | Set pulse-width level | update the global lamp pulse-width level using the low nibble |

### 4.2 Register windows

| Address range | Direction | Meaning |
|--------------|-----------|---------|
| **0x00..0x07** | write | lamp row image |
| **0x40..0x43** | read | switch snapshot bytes |
| **0xF0..0xF3** | read | diagnostics/status |

### 4.3 v1 transaction rules

- lamp updates are sent as a block write beginning at **0x00**
- switch reads are done as a block read beginning at **0x40**
- diagnostics reads are done beginning at **0xF0**
- there is **no commit register** in current v1
- there is **no explicit ready/heartbeat register** in current v1
- there is **no change-mask or event FIFO** in current v1
- the control board sends **intent**, not low-level scan-engine configuration

---

## 5. Timing Contract

### Matrix board internal timing
- main service loop target: **5 ms**
- scan/debounce target: **20 ms** stable debounce window
- one gameplay closure should only be reported once until that switch is released again
- lamp application latency target after a valid write: **within the next normal scan cycle**

### Control board polling and update timing
- switch polling target during active play: **every 10 to 20 ms**
- diagnostic polling target: **every 100 to 250 ms**, or on fault suspicion
- lamp writes: **on gameplay or mode changes**, with optional periodic refresh at **100 to 250 ms**
- control board should **not** spam lamp writes every matrix scan cycle

### Link supervision rule for MVP
- v1 uses **transaction success and diagnostics readability** as the health check
- if the control board cannot complete successful matrix transactions for about **1000 ms**, it should treat the matrix link as failed
- on link failure, the machine should fall back to the existing safe policy: inhibit gameplay continuation and avoid destructive output behavior

---

## 6. Control-Side Interpretation Rules

- the control board treats the **latest valid debounced switch snapshot** as authoritative
- duplicate or repeated reads of the same active bit are harmless and ignored idempotently
- release events do not need a separate wire-level event message in v1; they are inferred by comparing successive snapshots
- scoring logic fires from **new closures**, not from a held switch remaining active across multiple polls

---

## 7. Safe-State Rules

- if system is disabled, the matrix board must not actively drive the lamp matrix
- if output is disabled, lamp outputs remain off even if lamp row RAM is nonzero
- invalid or unsupported future protocol ideas must not disturb the last known safe runtime state in v1
- the matrix board remains timing-focused and should avoid embedding higher-level game logic

---

## 8. Current Bench-Implementation Note

The up-to-date matrix-board repo may still use direct local firmware routines, GPIO activity, and shift-register hardware during bring-up. That is acceptable and expected during bench validation.

The important architectural point is this:
- **implementation can evolve freely**
- **the control-side contract should stay simple**
- **HT16K33-style behavior is still the preferred abstraction**

---

## 9. Recommended Bring-Up / Alignment Order

1. verify safe power-up and boot behavior on the matrix board
2. validate the local scan/multiplex firmware on the bench
3. align the shared firmware to the agreed `0x24` behavior contract
4. write known patterns into **0x00..0x07** and verify lamp mapping
5. read **0x40..0x43** while actuating switches and confirm closures map correctly
6. read **0xF0..0xF3** and confirm status and pulse level
7. tune row/column polarity or mapping only after the contract above is verified

---

## 10. Lock Summary

For the preferred first aligned split-board firmware:
- **behavior contract = HT16K33-style I2C peripheral model**
- **control board responsibility = send lamp intent and read stable switch/diagnostic state**
- **matrix board responsibility = own the low-level implementation and timing details**
