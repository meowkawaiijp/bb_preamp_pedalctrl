# Development Guide

## Hardware

- **MCU**: ESP32-C3 (Seeed XIAO ESP32C3)
- **LCD**: AQM1602Y (ST7032 compatible, 16x2 character, I2C)
- **Digital potentiometer**: AD5263 (4-ch, 256-step, SPI bit-bang)
- **Shift register input**: 74HC165 (buttons + encoders)
- **Shift register output**: 74HC595 (relay, LEDs, 7-segment)
- **7-segment LED**: Common-anode, shows active preset number

## Build / Flash (Arduino IDE)

1. Install the **ESP32 Arduino Core** (Espressif Systems) via Board Manager.
2. Install the following libraries via Library Manager:
   - **MIDI Library** (Francois Best) -- MIDI I/O
   - **ST7032** -- I2C LCD driver for AQM1602Y
3. Open `BB_PREAMP_PedalCtrl.ino` in Arduino IDE.
4. Select board: **XIAO_ESP32C3** (or any ESP32-C3 board).
5. Upload.

## File Structure

```
BB_PREAMP_PedalCtrl.ino   Main sketch (setup / loop)
BB_PREAMP_PedalCtrl.h     Global defines, types, and constants
lcd_handler.ino/.h        LCD display rendering for all modes
ui_handler.ino/.h         Button and encoder UI logic
midi_handler.ino/.h       MIDI Program Change handler
web_server.ino/.h         HTTP request handlers (root, /preset)
index_html.ino            HTML template for WebUI (embedded string)
myUtil.ino/.h             Utility functions (LCD helpers, EEPROM, SPI, presets)
mySPI.ino/.h              Low-level shift-register I/O (HC165 read, HC595 write)
debug.ino                 Debug / test helpers
debug.cfg                 OpenOCD configuration for ESP32-C3
debug_custom.json         GDB attach configuration
```

## Architecture Overview

```
┌──────────────────────────────────────────────────────────┐
│                      setup()                             │
│  Init LCD -> Load EEPROM -> Connect WiFi -> Start HTTPD  │
│  -> Init MIDI -> Start HW Timer (1 kHz) -> Init state   │
└──────────────────────────────────────────────────────────┘
                          │
                          v
┌──────────────────────────────────────────────────────────┐
│                       loop()                             │
│                                                          │
│  [HW Timer ISR @ 1 kHz]                                  │
│    ReadHC165()  ──> raw 8-bit shift register data        │
│    HandleEncoder()  ──> rotation direction detection     │
│    Rotate7Seg()  ──> WiFi animation tick                 │
│                                                          │
│  [Main loop]                                             │
│    ParseSdiData()  ──> button + encoder state update     │
│    HandleSw()  ──> short-press / long-press detection    │
│    HandleEncoder()  ──> rotation direction (main loop)   │
│    UiButtonHandler()  ──> mode transitions, actions      │
│    UiEncoderHandler() ──> parameter changes              │
│    HandleMidi()  ──> MIDI Program Change                 │
│    UpdateEntireLcdDisplay()  ──> redraw LCD              │
│    SendMyHC595Serial()  ──> update 7-seg, relay, pots   │
│    httpServer.handleClient()  ──> WebUI requests         │
└──────────────────────────────────────────────────────────┘
```

## WebUI

When WiFi is enabled and connected, the device serves a web interface:

- **URL**: `http://bbp.local/` (mDNS) or the IP address shown on the LCD.
- **Preset editor**: 10 presets with Volume, Gain, Treble, Bass sliders.
- **Endpoint**: `GET /preset?v0=25&g0=50&t0=0&b0=0&...` updates presets.

The HTML is stored as C string literals in `index_html.ino`. Dynamic
preset values are injected by `CreateIndexHtml()` in `web_server.ino`.

## EEPROM

- Settings and presets are stored in EEPROM via the ESP32 `EEPROM.h` library.
- A signature string (`"BBPRE100"`) validates stored data on boot.
- All loaded values are range-checked by `CheckAndCorrectAppParams()` and
  `CheckAndCorrectPreset()`.
- `save_eeprom()` must be followed by `EEPROM.commit()` (handled internally).

## OTA Update

The firmware includes `HTTPUpdateServer`, allowing over-the-air updates
through the web interface when WiFi is connected.

## Debugging

- `debug.ino` contains hardware-level debug functions (`Debug_Pulse_D3`,
  `Debug_EncoderValue`, `Debug_All_IO`).
- `debug.cfg` + `debug_custom.json` provide OpenOCD/GDB configurations
  for JTAG debugging via the ESP32-C3 built-in USB-JTAG adapter.
- Set `debug_output_to_html = true` in `BB_PREAMP_PedalCtrl.ino` to
  append debug messages to the WebUI HTML output.
