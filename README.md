# Presettable BB-Preamp Controller

Arduino-based pedal controller for the BB-Preamp, featuring a 16x2 LCD UI,
10 tone presets, MIDI control, and a WiFi web editor.

More info: http://toshi.life.coocan.jp/review/en_diy_arduino_bbpreamp.html

## Features

- **10 Presets** -- Volume, Gain, Treble, Bass per preset
- **16x2 LCD** -- Intuitive menu system with cursor navigation
- **Rotary encoders** -- One for parameter editing, one for preset selection
- **7-segment LED** -- Shows active preset number (dot = unsaved changes)
- **Foot switch** -- Cycle presets or toggle effect on/off
- **MIDI** -- Program Change to recall presets remotely
- **WiFi WebUI** -- Edit all presets from a browser (`http://bbp.local/`)
- **EEPROM** -- Settings and presets persist across power cycles
- **OTA update** -- Firmware updates over WiFi

## Hardware

| Component | Role |
|-----------|------|
| Seeed XIAO ESP32C3 | Main MCU |
| AQM1602Y (ST7032) | 16x2 character LCD (I2C) |
| AD5263 | 4-channel digital potentiometer (Volume, Gain, Treble, Bass) |
| 74HC165 | Parallel-in shift register (buttons + encoders) |
| 74HC595 | Serial-out shift register (relay, LEDs, 7-segment) |

See `docs/presettable_bbp_schematic.pdf` for the full circuit schematic.

## Quick Start

1. Install [Arduino IDE](https://www.arduino.cc/en/software) with the
   **ESP32 Arduino Core** (Espressif Systems).
2. Install libraries: **MIDI Library**, **ST7032**.
3. Open `BB_PREAMP_PedalCtrl.ino`, select board **XIAO_ESP32C3**, and upload.
4. On first boot the LCD shows "EEPROM Load ERR" (expected) --
   configure WiFi SSID/PASS via the LCD menu, or use the pedal in WiFi-off mode.

## LCD Modes

| Mode | Screen | Description |
|------|--------|-------------|
| PLAY | `VOL GAIN TRE BAS` | Main screen -- adjust parameters, change presets |
| SETTINGS | `SET MIDI:.. MX:.. WiFi:.. FSW:..` | System settings |
| MENU | `MENU: PLAY SET INFO SSID PASS` | Mode selection (Enter long-press) |
| INFO | `http://bbp.local` | WiFi connection details |
| Edit SSID/PASS | Character editor | WiFi credential editing |
| Confirmations | `No / Yes` dialogs | Save preset, apply WiFi changes, reboot |

Full LCD reference: [docs/LCD_UI.md](docs/LCD_UI.md)

## WebUI

When WiFi is enabled and connected, open `http://bbp.local/` to access the
preset editor with slider controls for all 10 presets.

## Documentation

- [docs/LCD_UI.md](docs/LCD_UI.md) -- LCD screen reference
- [docs/CONFIGURATION.md](docs/CONFIGURATION.md) -- Parameter ranges, EEPROM layout, pin map
- [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) -- Build instructions, architecture, file structure
