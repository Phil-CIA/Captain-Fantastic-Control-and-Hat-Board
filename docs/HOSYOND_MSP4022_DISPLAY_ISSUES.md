# Hosyond MSP4022 4.0 SPI TFT Display Bring-up and Driver Issues

**Status:** Working draft  
**Last Updated:** 2026-04-18  
**Purpose:** Capture the known and likely issues involved in driving the Hosyond MSP4022 4.0 inch SPI TFT module in the Captain Fantastic bench environment, using both project evidence and external reports.

---

## 1. Display under evaluation

The display being discussed is sold as:
- **Brand:** Hosyond
- **Model:** MSP4022
- **Advertised panel:** 4.0 inch TFT
- **Advertised resolution:** 480 x 320
- **Advertised interface:** 4-wire SPI
- **Advertised TFT controller:** ST7796S
- **Advertised touch controller:** XPT2046 resistive touch
- **Other onboard feature:** microSD socket

This document should be treated as a practical bring-up reference, not a final datasheet substitute.

---

## 2. Confirmed evidence already gathered

### 2.1 From the earlier project work
The related bring-up work in the Development-Station-Power-Supply repo confirms the following:
- the display was being driven from an **ESP32 dev-board path** using **PlatformIO + Arduino + TFT_eSPI**
- a direct-wired pin map was used during earlier bench work:
  - **CS = 4**
  - **DC = 16**
  - **RST = 17**
  - **SCLK = 18**
  - **MISO = 19**
  - **MOSI = 23**
- the **full `display.init()` path could hang or stall boot** on the problematic setup
- a lower-level **write-only SPI fallback path** remained usable enough to continue bring-up and diagnostics
- multiple driver variants had already been considered during previous troubleshooting

### 2.2 From the product listing and public reports
Public descriptions and customer reports for this specific product family indicate:
- the 4.0 inch unit is sold as **ST7796S**, but some users report behavior that matches **ILI9488** instead
- common symptom reports include:
  - **white screen**
  - **partial drawing or clipping**
  - **wrong rotation**
  - **incorrect colors**
  - **touch instability or calibration trouble**

This does **not** prove that every MSP4022 module uses ILI9488, but it is a strong enough pattern that the controller should be treated as **unverified until confirmed on the bench**.

### 2.3 PCB-attached constraint and documentation risk
The display is now attached to a PCB, so the next verification step is **not** to keep moving wires around. The priority is to verify that the firmware signal definitions still match the schematic labels and PCB net mapping.

This matters because the related power-supply project already showed that a board can be made to work after code-side adjustments while the documentation lags behind. In practice, that means a past workaround may exist in code even if it was not captured clearly enough in the repo at the time.

A concrete lesson from that earlier work is that signal interpretation can be corrected in firmware after bench evidence reveals the first assumption was wrong. That is the exact failure mode we now want to prevent for this PCB-mounted display path.

**Recovered repo evidence now points to a real mapping split:**
- the PCB display-board notes used a local TFT bus mapped around GPIO10 through GPIO13 plus reset on GPIO19
- the later dev-board bring-up notes used TFT control on GPIO4, GPIO16, and GPIO17 with SPI on GPIO18, GPIO19, and GPIO23

That does not prove the current board is wrong, but it does prove the repo has contained **two different display signal interpretations** during bring-up history.

A dedicated worksheet for that review now exists here:
- [docs/DISPLAY_SIGNAL_VERIFICATION_WORKSHEET.md](docs/DISPLAY_SIGNAL_VERIFICATION_WORKSHEET.md)

---

## 3. Highest-probability issue list

| Symptom | Most likely cause | Confidence | Notes |
|---|---|---|---|
| White or blank screen | wrong controller driver, failed init sequence, or pin-map drift between PCB and firmware | High | Most common failure pattern seen with mislabeled TFT modules |
| Boot stalls during TFT init | library/controller mismatch, bad reset timing, readback problem, or wrong control pins | High | Seen in the related ESP32 bring-up repo |
| Partial graphics, clipping, or wrong resolution | wrong controller family or incorrect panel dimension/rotation config | High | Typical when using ST7796 settings on an ILI9488-like panel |
| Wrong colors or odd color depth | wrong pixel format or MADCTL settings | Medium | Often appears after a near-correct but not fully matched init |
| Screen lights but shows no useful image | backlight on, TFT logic not initialized | High | Easy to mistake for a working display path |
| Touch behaves badly or causes instability | XPT2046 SPI sharing, CS handling, or calibration issue | Medium | Touch should be deferred until TFT-only bring-up is stable |
| Works on 3.3 V ESP32 but not on 5 V Arduino | logic-level mismatch | High | Many reports indicate the module expects 3.3 V logic even if power input can accept 5 V |

---

## 4. Core issue categories

### 4.1 Controller ambiguity: ST7796S vs ILI9488
This is currently the **number one risk**.

The sales listing and many example packages describe the display as **ST7796S**, but multiple reports for the 4.0 inch version say the actual behavior matches **ILI9488** better. When the wrong driver is used, the result is often one or more of the following:
- white screen
- mirrored or rotated image
- partial frame updates
- bad color rendering
- successful backlight but no usable graphics

**Working assumption for bench work:** do not trust the listing alone; treat the controller as unknown until verified.

### 4.2 Full-library init may fail even when low-level SPI wiring is alive
The related project history shows an important distinction:
- the high-level library path could hang in `display.init()`
- the lower-level write-only SPI path could still produce useful behavior

That means a totally dead screen is **not** the only interpretation. It may be that:
- SPI wiring is mostly correct
- the reset sequence needs adjustment
- MISO/readback is unreliable
- the chosen controller init block is wrong for the actual panel

### 4.3 Logic-level and voltage issues
The module is often marketed as easy to use with Arduino, but in practice:
- the logic interface behaves most safely as a **3.3 V logic device**
- direct 5 V GPIO drive is risky and can cause white-screen or unreliable-SPI behavior
- power input may tolerate 5 V on the module, but that does **not** guarantee 5 V-safe signal pins

For the ESP32 path, this is less of a problem because the ESP32 uses 3.3 V logic natively.

### 4.4 Backlight / PWM confusion
A screen can appear “dead” or “blank” for two opposite reasons:
- the backlight is off, so a valid image is invisible
- the backlight is on, but the TFT controller never completed init, so all you see is a lit blank panel

Some users report that the LED/backlight pin needs explicit handling. This should be verified on the bench and documented with the actual module revision.

### 4.5 Reset polarity and timing sensitivity
The earlier repo work shows that reset handling mattered enough to deserve custom reset sequencing and guarded bring-up.

Potential failure modes include:
- reset line not actually connected where expected
- reset held at the wrong idle polarity
- reset pulse too short or too early
- library assumes a reset topology that does not match the module wiring

### 4.6 Shared SPI bus side effects from touch and SD
The module includes resistive touch and SD on the same local SPI family. If their chip-select behavior is not kept inactive during TFT-only testing, they can interfere with bring-up.

A high-risk detail recovered from the related project notes is that touch-related pins on the PCB mapping may overlap with GPIO numbers that were also used elsewhere during earlier bring-up work. That creates a real possibility that a code-side workaround was made during testing and not fully folded back into the board-level documentation.

**Practical rule:** first prove **display-only** operation. Add touch and SD later.

### 4.7 ESP32-C6 limitations are secondary, not primary
The ESP32-C6 is not the best heavy-GUI target, but for simple static screens it is not the main reason a white screen appears. The most likely blockers are still:
- controller mismatch
- init sequence mismatch
- wiring/reset/backlight problems

So the first phase should stay focused on **getting stable pixels on screen**, not on GUI architecture.

---

## 5. Recommended bench troubleshooting order

Use this order from fastest/highest-value checks to deeper investigation.

### Step 1 — Confirm the physical baseline
- verify **VCC**, **GND**, **CS**, **DC**, **RST**, **SCLK**, **MOSI**, and **MISO** continuity where the PCB still makes that possible
- for the PCB-attached version, compare the code definitions against the schematic labels before changing firmware assumptions
- explicitly check whether any GPIO21 or GPIO22 usage in the firmware conflicts with touch-related board assignments from the schematic path
- verify the last known direct-wire baseline only as a historical reference:
  - CS=4
  - DC=16
  - RST=17
  - SCLK=18
  - MISO=19
  - MOSI=23
- confirm the display is actually receiving power and the backlight is behaving as expected

### Step 2 — Prove raw SPI write behavior before full GUI work
- send simple full-screen color fills only
- avoid touch and SD during this step
- start at a conservative SPI clock and only raise speed after stable behavior is confirmed
- use the detailed **pin-by-pin checklist** in Section 6 before changing drivers or libraries

### Step 3 — Treat the controller as unknown until proven
Recommended driver trial order for this module family:
1. **ILI9488**
2. **ST7796S / ST7796**
3. **ILI9486**

Only test **one** driver assumption at a time and document the observed symptom for each.

### Step 4 — If full init hangs, fall back to write-only bring-up
If the library blocks in `display.init()`:
- keep the serial console alive
- keep the heartbeat alive
- use low-level wake, color fill, and address-window tests first
- defer controller-ID readback assumptions until the panel is visibly reacting

### Step 5 — Add touch only after display-only bring-up is solid
Do not spend time calibrating XPT2046 until the TFT is already drawing correctly.

### Step 6 — Record evidence immediately
When the display reacts differently under one driver or one reset path, save:
- photo of the screen result
- exact driver define used
- pin map used
- SPI speed used
- whether touch/SD were connected

---

## 6. Pin-by-pin bring-up checklist

This checklist now serves two purposes:
- verifying the original direct-wire assumptions
- verifying that the **PCB-attached implementation still matches the code definitions**

For a structured code-versus-schematic audit, use the companion worksheet:
- [docs/DISPLAY_SIGNAL_VERIFICATION_WORKSHEET.md](docs/DISPLAY_SIGNAL_VERIFICATION_WORKSHEET.md)

This checklist assumes the currently confirmed direct-wire bench baseline:
- **CS = 4**
- **DC = 16**
- **RST = 17**
- **SCLK = 18**
- **MISO = 19**
- **MOSI = 23**

| Signal | Purpose | What to verify first | Common failure symptom |
|---|---|---|---|
| **VCC** | Module power input | confirm the module is getting the intended supply and that the supply does not sag on startup | no backlight, unstable boot, random resets |
| **GND** | Common reference | verify low-resistance ground continuity between ESP32 and display | white screen, inconsistent SPI behavior, touch noise |
| **CS** | TFT chip select | make sure it reaches the correct TFT pin and idles inactive when not selected | no response or bus contention |
| **DC** | Data/command select | verify it toggles between command and pixel writes | lit panel with no valid graphics or scrambled output |
| **RST** | TFT hardware reset | verify polarity, pulse timing, and actual continuity to the module reset pin | stuck white screen or init hang |
| **SCLK** | SPI clock | verify activity during raw color-fill tests and start at a conservative rate | no image update or unstable drawing |
| **MOSI** | SPI data to TFT | verify continuity and visible data activity during test patterns | blank screen or corrupted graphics |
| **MISO** | TFT readback path | confirm it is not shorted or floating into nonsense during ID reads | init stalls, bad ID reads, read-command failures |
| **LED / PWM** | Backlight control | verify whether the backlight needs explicit enable or PWM drive | display appears dead even though logic may be alive |
| **TOUCH_CS** | XPT2046 chip select | keep it inactive during TFT-only bring-up | touch bus interference or odd SPI behavior |
| **SD_CS** | microSD chip select | keep it inactive during TFT-only bring-up | SPI contention and random display failures |

### Fast bench sequence using the checklist
1. power only: confirm backlight and supply stability
2. add **GND + CS + DC + RST + SCLK + MOSI** and try raw color fills
3. add **MISO** only after the write-only path reacts predictably
4. leave **touch** and **SD** disabled until the TFT is drawing correctly
5. record the exact symptom after each change instead of changing multiple variables at once

---

## 7. Recommended implementation direction for this project

For this project, the best near-term approach is:
- keep the display effort **bench-first and evidence-first**
- begin with **display-only** tests on the direct ESP32 wiring path
- prefer a proven simple stack such as **TFT_eSPI** or a low-level SPI test before full LVGL layering
- assume that the listing may be wrong until the actual controller behavior is confirmed
- if write-only mode works while full init does not, continue documenting that distinction rather than treating the whole screen path as dead

---

## 8. Known-good and likely-good starting assumptions

### Known-good enough to preserve from earlier work
- direct-wired ESP32 dev-board bench path
- PlatformIO + Arduino environment
- TFT_eSPI-based experimentation
- guarded or reduced-risk bring-up approach instead of assuming `display.init()` is safe

### Likely-good working rules
- use **3.3 V logic**
- start with **low SPI speed**
- keep **touch and SD disabled** during first tests
- verify **RST** behavior explicitly
- try **ILI9488** early if ST7796 init gives white-screen or partial-draw behavior

---

## 9. Open items to confirm on the actual hardware

The following still need direct bench confirmation on the user’s exact module:
- actual TFT controller marking, if visible
- whether the module revision really matches ST7796S or behaves like ILI9488
- whether the backlight pin needs explicit enable or PWM drive
- whether the current unit still works with the earlier direct-wire setup
- whether touch and SD interfere when left connected during TFT-only testing

---

## 10. Practical conclusion

At this point, the most likely explanation for the current white-screen path is **not** “the display is simply dead.” The stronger working hypothesis is:

1. the module/controller identity is ambiguous
2. the high-level init path is too optimistic for this hardware
3. a careful write-only bring-up path plus driver re-validation is the correct next step

That is the implementation path this project should follow first.
