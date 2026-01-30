# Development notes

## Build/flash (Arduino IDE)
1. Install the ESP32 Arduino core.
2. Install libraries referenced by the includes:
   - MIDI library
   - ST7032 LCD library
3. Open BB_PREAMP_PedalCtrl.ino.
4. Select an ESP32C3 board (for example Seeed XIAO ESP32C3) and upload.

## Web UI
- When WiFi is enabled and connected, open http://bbp.local/
- Submitting /preset updates the preset table.

## EEPROM
- Settings and presets are stored in EEPROM and validated on load.
