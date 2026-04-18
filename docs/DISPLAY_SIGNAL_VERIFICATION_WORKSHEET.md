# Display Signal Verification Worksheet

**Status:** Active bench worksheet  
**Last Updated:** 2026-04-18  
**Purpose:** Verify that the TFT-related firmware signal definitions still match the actual PCB schematic labels and net mapping for the permanently attached display assembly.

---

## 1. Why this worksheet exists

The display is now attached to a PCB, so this is no longer a loose-wire experiment where pins can be moved around freely during testing.

That changes the debugging priority:
- we must verify that the **code signal definitions** still match the **schematic labels and PCB net names**
- we must assume a past bring-up workaround may have been rolled into firmware without being documented clearly enough
- we must prove the mapping from repo evidence, not memory

---

## 2. Historical caution from the related power-supply work

The related display bring-up history already showed that a working firmware mapping can drift away from the first assumed wiring interpretation.

One concrete example from that broader bring-up effort was a bus-mapping correction that had to be captured after the fact once the board behavior proved the original assumption wrong. That is exactly the kind of drift this worksheet is meant to catch early for the display path.

That means this project should explicitly check for:
- swapped signal interpretation
- hidden pin overrides in build flags or setup headers
- reset polarity changes
- write-only fallback behavior that bypassed a deeper mapping problem

**Rule:** if code and schematic disagree, the repo must record the difference immediately.

---

## 3. Current bench assumption to compare against

The earlier direct-wire baseline that was known to work or partially work used:
- **CS = 4**
- **DC = 16**
- **RST = 17**
- **SCLK = 18**
- **MISO = 19**
- **MOSI = 23**

This baseline is a reference only. It is **not automatically the truth** for the PCB-attached version.

---

## 4. Code-to-schematic verification table

Fill this out from the schematic, PCB labels, and current firmware definitions.

### Historical values already recovered from the related project
The following rows are partially prefilled from the earlier bring-up notes so the review can start from evidence instead of memory.

| Schematic label | Function | MCU GPIO per schematic | Historical direct-wire compiled value | Initial comparison | Notes |
|---|---|---:|---:|---|---|
| TFT_CS | TFT chip select | 10 | 4 | MISMATCH | likely dev-board workaround or alternate wiring path |
| TFT_DC | Data / command | 18 | 16 | MISMATCH | verify whether the PCB net was bypassed during bench bring-up |
| TFT_RST or TFT_HW_RST | Hardware reset | 19 | 17 | MISMATCH | very high priority; reset timing and polarity already mattered historically |
| TFT_SCLK | SPI clock | 12 | 18 | MISMATCH | board-level path and dev-board path appear different |
| TFT_MOSI | SPI write data | 13 | 23 | MISMATCH | check for direct-wire bring-up path that bypassed PCB routing |
| TFT_MISO | SPI readback | 11 | 19 | MISMATCH | may explain readback or init instability |
| TFT_LED or PWM | Backlight enable / PWM | 20 | unknown | UNCLEAR | verify whether firmware ever drove this explicitly |
| TOUCH_CS | XPT2046 chip select | 22 | unknown | UNCLEAR | schematic assignment overlaps with historical non-display usage in related bring-up |
| TOUCH_IRQ | Touch interrupt | 21 | unknown | UNCLEAR | schematic assignment overlaps with historical non-display usage in related bring-up |

### Working table for current board verification

| Schematic label | Function | MCU GPIO per schematic | Firmware symbol or build flag | Actual compiled value | Match? | Notes |
|---|---|---:|---|---:|---|---|
| TFT_CS | TFT chip select | ___ | ___ | ___ | ___ | compare against historical 10 vs 4 mismatch |
| TFT_DC | Data / command | ___ | ___ | ___ | ___ | compare against historical 18 vs 16 mismatch |
| TFT_RST or TFT_HW_RST | Hardware reset | ___ | ___ | ___ | ___ | high priority check |
| TFT_SCLK | SPI clock | ___ | ___ | ___ | ___ | compare against historical 12 vs 18 mismatch |
| TFT_MOSI | SPI write data | ___ | ___ | ___ | ___ | compare against historical 13 vs 23 mismatch |
| TFT_MISO | SPI readback | ___ | ___ | ___ | ___ | compare against historical 11 vs 19 mismatch |
| TFT_LED or PWM | Backlight enable / PWM | ___ | ___ | ___ | ___ | verify explicit enable behavior |
| TOUCH_CS | XPT2046 chip select | ___ | ___ | ___ | ___ | watch for GPIO22 overlap risk |
| TOUCH_IRQ | Touch interrupt | ___ | ___ | ___ | ___ | watch for GPIO21 overlap risk |
| SD_CS | microSD chip select | ___ | ___ | ___ | ___ | verify it stays inactive during TFT-only test |
| 3V3 or 5V rail | Display power rail | ___ | n/a | n/a | ___ | confirm actual rail used on PCB |
| GND | Ground reference | ___ | n/a | n/a | ___ | confirm common return path |

### Provisional repo-based conclusion
From the recovered evidence so far, the earlier display-board schematic path and the later dev-board bring-up path do **not** describe the same signal mapping.

**PCB schematic path from the related display-board notes:**
- TFT_CS = GPIO10
- TFT_DC = GPIO18
- TFT_RST = GPIO19
- TFT_SCLK = GPIO12
- TFT_MISO = GPIO11
- TFT_MOSI = GPIO13
- TFT backlight PWM = GPIO20
- TOUCH_CS = GPIO22
- TOUCH_IRQ = GPIO21
- SD_CS = GPIO15

**Historical bring-up firmware path recovered from the same broader work:**
- TFT_CS = 4
- TFT_DC = 16
- TFT reset driven from 17
- SPI bus = 18 / 19 / 23
- write-only mode became the stable path
- I2C eventually had to be corrected to SDA=22 and SCL=21

**Interpretation:**
- either the display was being proven on a direct dev-board wiring path that bypassed the PCB mapping
- or code-side overrides were introduced during bring-up and not fully written back into the board documentation

Until that difference is resolved against the current board revision, a mapping drift should be treated as a top-tier root-cause candidate.

---

## 5. What to inspect in firmware

Check all of the following before assuming the mapping is still correct:
- platform build flags
- any display setup header
- source-level pin defines
- reset polarity settings
- driver select defines
- any write-only bypass path

If one value differs from the schematic, write down:
1. whether it was deliberate
2. whether it was necessary to get the display working
3. whether the repo documentation was updated to match

---

## 6. Bench verification sequence

### Step 1 — Read the schematic literally
- do not trust memory
- write down the display connector labels exactly as shown
- write down the MCU GPIO tied to each label

### Step 2 — Read the current firmware literally
- find the actual signal definitions in the current build path
- note any alternate names, overrides, or conditional defines
- verify reset polarity and any special-case backlight handling

### Step 3 — Compare line by line
Mark each signal as one of:
- **MATCH**
- **MISMATCH**
- **UNCLEAR / needs bench proof**

### Step 4 — Probe only what matters most first
Highest priority signals:
1. power
2. ground
3. reset
4. CS
5. DC
6. SCLK
7. MOSI
8. backlight
9. MISO

> Special caution: the recovered historical notes suggest that GPIO21 and GPIO22 were used in another bring-up path while the display-board schematic used those same numbers for touch-related functions. Treat any 21/22 reuse as a deliberate item to verify, not as a safe assumption.

### Step 5 — Document the result in the repo
If a mismatch or workaround is found:
- update the display issues doc
- update this worksheet
- add the final known-good mapping to the repo notes

---

## 7. Symptoms that strongly suggest a mapping or setup mismatch

| Symptom | Likely interpretation |
|---|---|
| Backlight on but white screen | TFT logic not initializing, wrong driver, or bad reset / DC mapping |
| Boot hangs during display init | controller mismatch, reset issue, or bad readback path |
| Raw color fill works but normal library init does not | software config mismatch rather than total wiring failure |
| Touch starts failing when TFT is active | SPI bus sharing or CS handling problem |
| Only one driver family reacts at all | likely mislabeled or alternate controller on the module |

---

## 8. Result summary for this board

### Current status
- [x] historical repo evidence recovered and compared
- [ ] schematic reviewed against current code path
- [ ] all TFT signals confirmed on the real board
- [ ] reset behavior confirmed
- [ ] backlight behavior confirmed
- [ ] touch kept isolated during TFT-only test
- [ ] final known-good mapping written back into repo docs

### Final known-good mapping
- CS = ___
- DC = ___
- RST = ___
- SCLK = ___
- MISO = ___
- MOSI = ___
- LED/PWM = ___
- Touch CS = ___
- SD CS = ___

### Notes
- __________________________________________
- __________________________________________
- __________________________________________
