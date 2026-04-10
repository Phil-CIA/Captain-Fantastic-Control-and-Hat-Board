# Solenoid Driver Selection Note

This note captures the current design direction for the **base control-board high-side solenoid driver** ahead of the next PCB reorder.

## Problem summary
The current discrete high-side circuit built around:
- `IRFR5410`
- `BSS84`
- `BSS138`

is creating an unsafe / uncomfortable **`Vgs` margin problem** on the real **22–26 V** solenoid rail.

That makes the present discrete PMOS approach a poor choice for the next board revision.

## What the application actually needs
The replacement driver needs to match the real use case:
- **high-side switch** behavior
- **22–26 V** pinball solenoid rail
- **inductive pulse load**
- **ESP32-friendly logic interface**
- fast turnaround for a near-term board reorder
- robust behavior with current limiting / thermal protection preferred

## Candidate approaches

### 1) Keep the current discrete PMOS circuit
**Verdict:** reject for the next board revision.

**Reason**
- current gate-drive arrangement is the known issue
- adds avoidable validation risk right before reorder
- not the right place to spend more schedule if a protected smart switch already fits the job better

### 2) Return to a TIP125-style Darlington high-side stage
**Verdict:** historical reference only, not the preferred modern solution.

**Reason**
- this matches older TTL-era pinball design practice
- it is simple and familiar
- but it wastes more voltage, runs hotter, and gives less protection than a modern smart high-side switch

### 3) Use a protected smart high-side switch
**Verdict:** preferred direction.

**Reason**
- matches the real application better than the current discrete PMOS circuit
- removes the present `Vgs` concern from the design
- gives built-in protection for inductive load behavior
- fits the goal of getting the base board ready to reorder soon

## Evidence from candidate parts

### ST `VNQ7E100AJ`
From the ST product page:
- **quad smart high-side driver**
- **3 V / 5 V CMOS-compatible** interface
- built-in **load current limitation**
- **overtemperature shutdown**
- **overvoltage clamp**
- diagnostic / current-sense support

This is the same smart-switch family already used on the matrix-board work, which lowers project risk.

Source:
- `https://www.st.com/en/automotive-analog-and-power/vnq7e100aj.html`

### TI `TPS4HC120-Q1`
From the TI product page:
- **4-channel** smart high-side switch
- **3 V to 28 V** operating range
- **120 mΩ** typical on-resistance
- **2.5 A** listed per-channel current class
- **adjustable current limit**
- **inductive load compatibility**
- thermal shutdown and current monitoring

Source:
- `https://www.ti.com/product/TPS4HC120-Q1`

### TI `TPS1H200A-Q1`
From the TI product page:
- **single-channel** smart high-side switch
- **4 V to 40 V** operating range
- **200 mΩ** typical on-resistance
- **adjustable current limit**
- thermal shutdown
- explicitly listed for **inductive loads**

Source:
- `https://www.ti.com/product/TPS1H200A-Q1`

## Current recommendation
For the next base control-board revision:

1. **do not reuse the present discrete PMOS high-side stage**
2. move to a **smart high-side switch architecture**
3. proceed with a **single-channel device layout strategy** for the five solenoid outputs so routing and placement stay simple on this board
4. use **`TPS1H200A-Q1` as the selected baseline device** for the repeated output channels unless later bench data proves a mismatch

## Practical board-level direction
There are currently five named solenoid outputs in the firmware mapping (`S2` through `S6`).

The chosen board-level direction is now:
- **five single-channel smart high-side outputs**
- repeated per-channel layout instead of centering the design around one larger multi-channel package
- see `docs/SINGLE_CHANNEL_SOLENOID_LAYOUT.md` for the placement / routing intent

## Implementation note
A concrete starting-point hookup plan for the repeated channel block is now captured in `docs/TPS1H200_IMPLEMENTATION_NOTES.md` so the KiCad update can move from “direction” to “specific repeated block.”

## Remaining confirmation step
Before the final part number is frozen, confirm the worst-case coil demand for the target pinball coil:
- DC resistance
- expected pulse width
- whether the channel ever needs only a short pulse or anything closer to hold behavior

## Preliminary conclusion
The decision is now narrowed substantially:
- **architecture:** smart high-side switch
- **chosen baseline part:** `TPS1H200A-Q1`
- **not recommended:** current discrete PMOS stage
- **not preferred:** TIP125 Darlington return
- **preferred next action:** implement the repeated `TPS1H200A-Q1` channel block in the next schematic pass and confirm the coil envelope during bench validation
