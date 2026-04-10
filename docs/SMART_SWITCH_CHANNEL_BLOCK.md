# Single-Channel Smart High-Side Block

This note defines the repeated **per-channel schematic block** for the next base control-board respin.

## Scope
This block is intended to be copied for each solenoid output:
- `S2`
- `S3`
- `S4`
- `S5`
- `S6`

## Per-channel block contents
Each output channel should include:
- one **single-channel smart high-side switch**
- one MCU control input net
- one output net to the solenoid connector
- local supply decoupling
- optional fault / current-sense return to MCU if the selected device supports it

## Suggested net naming pattern
For each channel, use a clear repeated pattern such as:

### Supply / return
- `SOL_24V`
- `SOL_GND`

### Logic control
- `SOL_EN_S2`
- `SOL_EN_S3`
- `SOL_EN_S4`
- `SOL_EN_S5`
- `SOL_EN_S6`

### Outputs
- `SOL_OUT_S2`
- `SOL_OUT_S3`
- `SOL_OUT_S4`
- `SOL_OUT_S5`
- `SOL_OUT_S6`

### Optional diagnostics
- `SOL_FAULT_S2`
- `SOL_FAULT_S3`
- `SOL_FAULT_S4`
- `SOL_FAULT_S5`
- `SOL_FAULT_S6`

## Functional block sketch
Use the same structure for each channel:

`SOL_24V` -> smart high-side switch -> `SOL_OUT_Sx` -> connector / coil load

MCU GPIO -> input pin of smart switch

Optional:
- smart-switch fault / sense pin -> MCU input or test pad
- pull resistors only if required by the selected datasheet

## Local support parts
Per channel, place:
- local ceramic bypass capacitor close to the device supply pin(s)
- optional bulk support nearby per the final device datasheet and board grouping
- test point on the output or fault path if useful for bring-up

## Layout intent
- place each channel near the output it serves
- keep `SOL_24V` feed wide and direct
- keep the device-to-output trace short
- keep logic traces out of the high-current return path where practical
- give each device copper area for thermal spreading

## Firmware/control mapping
The current bring-up firmware names are already:
- `S2 / SD2`
- `S3 / SD3`
- `S4 / SD4`
- `S5 / SD5`
- `S6 / SD6`

That means the new schematic should preserve those control names where practical to reduce firmware churn.

## Current note
The exact smart-switch part number can still be finalized, but this block definition is now the intended repeated schematic pattern for the control-board respin.
