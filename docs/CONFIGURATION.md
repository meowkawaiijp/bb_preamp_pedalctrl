# Configuration and Parameter Ranges

This document lists every configurable parameter, its valid range, and where
it is used.  All ranges are enforced by `UpdateClampedInt()` /
`CheckAndCorrectAppParams()` on EEPROM load and by the UI encoder handlers.

---

## Tone Parameters (per-preset)

| Parameter | Min | Max | Default | LCD label | WebUI label |
|-----------|-----|-----|---------|-----------|-------------|
| volume    |  0  |  99 |   25    | VOL       | Volume      |
| gain      |  0  |  99 |   50    | GAIN      | Gain        |
| treble    | -50 | +50 |    0    | TRE       | Treble      |
| bass      | -50 | +50 |    0    | BAS       | Bass        |

These values are stored in `Type_PresetParams` and written to EEPROM as
part of the preset array (10 slots: preset 0-9).

Volume and Gain are mapped to AD5263 digital potentiometer steps:
`step = 255 * value / 99` (0-255).
Treble and Bass are mapped as:
`step = 255 * (value + 50) / 100` (0-255).

---

## System Parameters (global, saved to EEPROM)

| Parameter     | Field            | Min | Max | Default | LCD label |
|---------------|------------------|-----|-----|---------|-----------|
| Active preset | `presetNo`       |  0  |  9  |    0    | (7-seg)   |
| MIDI channel  | `midi_ch`        |  1  | 16  |    1    | MIDI:     |
| Preset max    | `preset_max`     |  2  |  9  |    9    | MX:       |
| WiFi enable   | `wifi_enable`    |  0  |  1  |    0    | WiFi:     |
| Foot SW mode  | `footsw_mode`    |  0  |  1  |    0    | FSW:      |
| Effect on/off | `onoff`          |  0  |  1  |    0    | (relay)   |

### Foot switch modes

| Value | Constant         | LCD display | Behaviour |
|-------|------------------|-------------|-----------|
|   0   | `FOOTSW_ALTERNATE` | Alt       | Toggle effect on/off per press |
|   1   | `FOOTSW_PRESET`    | Pst       | Cycle through presets 0..preset_max |

---

## WiFi Credentials

| Field     | Max length | Valid characters | LCD edit screen |
|-----------|-----------|------------------|-----------------|
| SSID      | 32 chars  | space, 0-9, a-z, A-Z, `_`, `-` | Edit SSID |
| Passphrase| 63 chars  | space, 0-9, a-z | Edit PASS |

Stored in `appParam.wifi_ssid[]` and `appParam.wifi_pass[]`.
Trailing spaces are trimmed (replaced with `\0`) when committing an edit.

---

## EEPROM Layout

```
Offset  Size                    Content
──────  ──────────────────────  ─────────────────────
0x000   10 bytes                Signature ("BBPRE100")
0x00A   sizeof(Type_AppParams)  System parameters
...     sizeof(Type_PresetParams) * 10  Preset array
```

The EEPROM signature is checked on boot.  If it does not match, the LCD
shows "EEPROM Load ERR / Wrong signature" for 3 seconds and factory
defaults are used (but not saved until the user explicitly saves).

---

## MIDI

- **Protocol**: MIDI 1.0 over hardware Serial (UART)
- **Supported message**: Program Change (0xCn)
- **Behaviour**: When a Program Change is received on the configured
  MIDI channel with data byte 0-9, the corresponding preset is loaded
  immediately.  Values outside 0-9 are ignored.

---

## Hardware Pin Assignments

| Function | Pin | Notes |
|----------|-----|-------|
| LED1     | D3  | Status / debug LED |
| AD5263 CLK | D8 | SPI clock for digital potentiometer |
| AD5263 SDI | D10 | SPI data for digital potentiometer |
| AD5263 CS  | D9 | SPI chip select |
| HC165 SDI  | D0 | Serial data input from shift register |
| HC165 CS   | D1 | Chip select for shift register |
| HC165 CLK  | D2 | Clock for shift register |
