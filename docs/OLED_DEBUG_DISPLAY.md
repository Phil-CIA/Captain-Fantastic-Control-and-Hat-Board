# OLED Debug Display Notes

A **.96-inch SSD1306-style I2C OLED** has been added to the control-board design to help with:
- boot messages
- troubleshooting status
- quick local diagnostics without depending only on serial output

## Current intent
For now, the OLED is treated as a **bring-up aid** for the new firmware baseline under `firmware/control-board/`.

## What is already in place
- the clean control-board `main.cpp` now scans the I2C bus during bring-up
- the firmware now targets the common **SSD1306** controller family directly
- expected debug OLED addresses are currently tracked as `0x3C` and `0x3D`
- serial boot output reports whether a likely OLED device is detected on the shared I2C bus
- when the OLED initializes successfully, it shows a simple local bring-up status message

## Current assumption
This is being treated as the common **128x64 `.96-inch` SSD1306-style module** unless later bench testing proves otherwise.

## Next step for richer on-screen text
Once the display is confirmed on the bench, the next step is to expand from simple bring-up text into more detailed boot logs, status pages, or troubleshooting readouts.

## Bring-up note
Keep the OLED on the normal low-voltage I2C side of the board during the current **5 V bench bring-up**. It fits well with the current troubleshooting-first development flow.
