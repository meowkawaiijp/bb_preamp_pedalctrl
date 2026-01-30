# File structure

## Root sketch
- BB_PREAMP_PedalCtrl.ino: setup/loop, boot sequence, WiFi, timers.
- BB_PREAMP_PedalCtrl.h: constants, types, and ranges.

## UI
- ui_handler.ino / ui_handler.h: button and encoder handling.
- lcd_handler.ino / lcd_handler.h: LCD rendering.

## MIDI
- midi_handler.ino / midi_handler.h: MIDI program change handling.

## Web
- web_server.ino / web_server.h: HTTP handlers and preset updates.
- index_html.ino: HTML template for the web UI.

## Hardware IO
- mySPI.ino / mySPI.h: shift registers, encoders, 7-seg output.
- myUtil.ino / myUtil.h: EEPROM, AD5263 control, helpers.

## Debug artifacts
- debug.ino: optional debug helpers (not used by default).
- debug.cfg, debug_custom.json, esp32c3.svd, esp32s3.svd: OpenOCD/GDB config.

## Documents
- docs/README.md: documentation index.
- docs/DEVELOPMENT.md: build notes.
- docs/CONFIGURATION.md: parameter ranges and settings.
