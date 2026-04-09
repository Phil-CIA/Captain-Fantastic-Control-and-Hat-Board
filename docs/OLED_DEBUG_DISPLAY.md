# OLED Debug Display Notes

A **256 x 128 I2C OLED** has been added to the control-board design to help with:
- boot messages
- troubleshooting status
- quick local diagnostics without depending only on serial output

## Current intent
For now, the OLED is treated as a **bring-up aid** for the new firmware baseline under `firmware/control-board/`.

## What is already in place
- the clean control-board `main.cpp` now scans the I2C bus during bring-up
- expected debug OLED addresses are currently tracked as `0x3C` and `0x3D`
- serial boot output will report whether a likely OLED device is detected on the shared I2C bus

## Next step for full on-screen text
To actually render boot logs or status text on the OLED, the next thing to confirm is the **exact controller / driver IC** used by the display module.

Examples of what this might be:
- `SSD1306`
- `SH1106`
- `SH1107`
- another large-format OLED controller

Once that is confirmed, the proper library and initialization code can be added cleanly.

## Bring-up note
Keep the OLED on the normal low-voltage I2C side of the board during the current **5 V bench bring-up**. It fits well with the current troubleshooting-first development flow.
