# Control ↔ Matrix Board Interface Contract (v1)

**Status:** Locked for current Rev 1 bench-alignment work  
**Last Updated:** 2026-04-18  
**Purpose:** Define the actual current interface between the control side and the up-to-date matrix-board repo/hardware.

This document supersedes the older idea of an HT16K33-style I2C peripheral at address `0x24`. The current matrix-board repo and ordered Rev 1 hardware use a **direct logic / shift-register control model**, not an implemented I2C register map.

---

## 1. Board Ownership Boundary

### Control side owns
- game state machine
- scoring and bonus logic
- audio behavior
- diagnostics policy
- persistence and operator settings
- high-level lamp intent decisions

### Matrix board owns
- local lamp-drive hardware execution
- switch-column sensing
- scan timing and boot-safe output behavior
- low-level bring-up diagnostics

### Explicit non-goal for current v1
- the matrix board does **not** presently expose a rich remote command/register API

---

## 2. Live Rev 1 Transport Model

- **Interface style:** direct 3.3V logic signals around a `74HC595`-style shift-register path
- **Matrix controller:** local `ESP32-C6` firmware on the matrix board
- **Lamp drive architecture:** `2x VNQ7E100AJTR` high-side row drivers plus low-side column sinks
- **Switch readback architecture:** `LMV393` comparator outputs read as logic-level column signals
- **Current debug/observability:** UART prints and probe-able test points

For the current repo state there is:
- **no live I2C slave address**
- **no fixed register window**
- **no commit/heartbeat/change-mask protocol**
- **no framed packet transport requirement**

---

## 3. External Signal Set

### 3.1 Control signals into the matrix board

| Signal | Direction at matrix board | Voltage | Purpose |
|---|---|---:|---|
| `SR_SCLK` | In | 3.3V | shift clock for the serial output path |
| `SR_LATCH` | In | 3.3V | latch/strobe for applying shifted output data |
| `SR_DATA0` | In | 3.3V | primary serial data line |
| `SR_DATA1` | In (optional) | 3.3V | optional second serial data line |
| `SR_OE_N` | In (recommended spare) | 3.3V | optional active-low global output enable |
| `UART_RX` | In (optional) | 3.3V | bring-up/debug serial input |
| `EN/RESET` | In (optional) | 3.3V | reset or enable control |

### 3.2 Signals provided back from the matrix board

| Signal | Direction at matrix board | Voltage | Purpose |
|---|---|---:|---|
| `SW_COL_0` | Out | 3.3V | switch-column readback |
| `SW_COL_1` | Out | 3.3V | switch-column readback |
| `SW_COL_2` | Out | 3.3V | switch-column readback |
| `SW_COL_3` | Out | 3.3V | switch-column readback |
| `UART_TX` | Out (optional) | 3.3V | bring-up/debug serial output |

### 3.3 Shared rails / references

- `+3V3_LOGIC`
- `+5V_SW` (local or provided, depending on the final harness choice)
- `GND`
- lamp supply rail per the matrix-board power-entry design

---

## 4. Lamp-Control Model

### 4.1 Physical matrix shape
- **8 lamp rows**
- **4 lamp columns**
- rows are driven high-side
- columns are sunk low-side

### 4.2 Current control behavior
- the current firmware shifts a **16-bit output pattern** to the attached output-expansion hardware
- the new state becomes visible when the latch line is pulsed
- the repo's bring-up scaffold currently walks a test bit pattern to validate wiring and output sequencing

### 4.3 Important mapping note
The exact bit-to-row/column assignment should be treated as a **bench-confirmed pinmap item**, not a presumed protocol constant. The current repo explicitly notes that bit order may need adjustment to match the schematic/harness.

---

## 5. Switch-Read Model

- the matrix board presently uses **4 logic-level switch-column outputs** (`SW_COL_0..3`)
- the current bring-up firmware samples them directly with GPIO reads
- there is **no packed byte snapshot published over I2C** in the live Rev 1 implementation
- if a control-side companion directly monitors these lines during bench work, polling every **10 to 20 ms** is still a reasonable target
- release and edge interpretation are currently inferred from successive logic samples unless and until a richer protocol is added later

---

## 6. Safe-State and Timing Expectations

### Safe state
- outputs should default to **all off** at boot
- no lamp should flash unintentionally before firmware has initialized the output path
- `/OE` and `/MR` boot behavior remain important hardware bring-up checks

### Timing
- the present bring-up firmware uses a slow visible walking-pattern cadence for validation
- the longer-term implementation target remains roughly a **5 ms** service cadence with about **20 ms** debounce behavior once the firmware matures
- these are current design targets, not yet a separate remote bus contract

---

## 7. Diagnostics and Bring-Up Visibility

Current Rev 1 visibility is intentionally simple:
- UART debug text from the local `ESP32-C6`
- direct probing of `SR_SCLK`, `SR_LATCH`, `SR_DATA0`, `SW_COL_0..3`, and the main rails
- one-row / one-column supervised lamp validation before full matrix stress testing

Structured diagnostic registers, fault bytes, and heartbeat flags are **future ideas only** unless later firmware explicitly implements them.

---

## 8. Not Part of the Live Rev 1 Contract

The following items should **not** be assumed implemented today:
- I2C address `0x24`
- register windows such as `0x00..0x07`, `0x40..0x43`, or `0xF0..0xF3`
- command bytes such as `0x20`, `0x21`, `0x80`, `0x81`, or `0xE0..0xEF`
- explicit ready registers, commit registers, or event FIFOs
- mandatory per-lamp brightness/PWM wire protocol

---

## 9. Recommended Bring-Up / Integration Order

1. verify `3.3V`, `5V`, lamp supply, and ground behavior on the matrix board
2. confirm the `ESP32-C6` boots and emits expected debug output
3. probe `SR_SCLK`, `SR_LATCH`, and `SR_DATA0` while the bring-up firmware runs
4. confirm outputs remain off until intentionally driven
5. actuate known switches and verify `SW_COL_0..3` logic behavior and polarity
6. lock the real row/column mapping after bench confirmation
7. only after that, decide whether a future higher-level host protocol is still needed

---

## 10. Lock Summary

For the current real Rev 1 matrix-board implementation:
- **interface = direct logic signals for shift-register control plus 4 switch-column readback lines**
- **local controller = ESP32-C6 with UART-oriented bring-up diagnostics**
- **older I2C register-map ideas are deferred and must not be treated as already implemented**
