# LCD UI Reference

LCD: AQM1602Y (ST7032 compatible) -- 16 columns x 2 rows.

This document describes every screen the LCD can display.
`>` is the selection cursor (moves with Left/Right buttons).
`_` denotes a blinking hardware cursor (ST7032 blink mode).

---

## Mode 0 -- PLAY

The main operating screen. Shows the four tone parameters and allows
editing with the parameter encoder.

```
Col: 0123456789012345
L0:  VOL GAIN TRE BAS
L1:  >25  50 *+00 +00
```

| Area | Cols (L1) | Description |
|------|-----------|-------------|
| VOL  | 1-2       | Volume (00-99) |
| GAIN | 5-6       | Gain (00-99) |
| TRE  | 9-11      | Treble (-50 to +50, signed 3-char) |
| BAS  | 13-15     | Bass (-50 to +50, signed 3-char) |
| `>`  | 0, 4, 8, or 12 | Selection cursor (one at a time) |
| `*`  | 7         | Shown when preset has unsaved changes |

### Controls

| Input | Action |
|-------|--------|
| Left button | Move cursor left |
| Right button | Move cursor right |
| Param encoder | Adjust selected parameter |
| Preset encoder | Change preset number (loads preset) |
| Enter (short) | Open "Save preset?" confirmation (only when modified) |
| Enter (long) | Open MENU screen |
| Foot switch | Cycle preset (Preset mode) or toggle on/off (Alternate mode) |

### External indicators

- **7-segment LED**: displays the active preset number (0-9).
  A dot appears on the 7-seg when the preset has unsaved changes.
- **Relay LED**: lit when the effect is ON.

---

## Mode 1 -- SETTINGS

System-level parameters. Saved to EEPROM on Enter (short-press).

```
Col: 0123456789012345
L0:  SET>MIDI:01 MX:9
L1:   WiFi:On FSW:Pst
```

| Item | Label | Cols (value) | Range | Description |
|------|-------|--------------|-------|-------------|
| MIDI ch | `MIDI:` | 9-10, L0 | 01-16 | MIDI receive channel |
| Preset Max | `MX:` | 15, L0 | 2-9 | Highest preset number reachable by foot switch |
| WiFi | `WiFi:` | 6-8, L1 | On/Off | Enable or disable WiFi (requires reboot) |
| Foot SW | `FSW:` | 13-15, L1 | Pst/Alt | Foot switch behaviour |

Cursor positions: col 3 (MIDI), col 11 (MX), col 0 (WiFi), col 8 (FSW).

### Controls

| Input | Action |
|-------|--------|
| Left / Right | Move selection cursor |
| Param encoder | Change selected setting value |
| Enter (short) | Save to EEPROM and return to PLAY (or reboot confirmation if WiFi changed) |

---

## Mode 2 -- MENU (Mode Change)

Reached by long-pressing Enter in PLAY mode.

```
Col: 0123456789012345
L0:  MENU: PLAY  SET
L1:   INFO SSID  PASS
```

| Item | Description | Destination mode |
|------|-------------|------------------|
| PLAY | Return to play | MODE_PLAY |
| SET  | Open settings | MODE_SETTINGS |
| INFO | Show WiFi connection info | MODE_SHOW_URI |
| SSID | Edit WiFi SSID | MODE_EDIT_SSID |
| PASS | Edit WiFi passphrase | MODE_EDIT_PASS |

Cursor positions: col 6 (PLAY), col 12 (SET), col 0 (INFO), col 5 (SSID), col 11 (PASS).

### Controls

| Input | Action |
|-------|--------|
| Left / Right | Move selection cursor |
| Enter (short) | Enter selected sub-mode |

---

## Mode 3 -- WiFi INFO

Displays the mDNS URL and IP address.

```
Col: 0123456789012345
L0:  http://bbp.local
L1:  192.168.1.42
```

If WiFi is not connected:

```
L1:  (No connection)
```

### Controls

| Input | Action |
|-------|--------|
| Enter (short) | Return to PLAY |

---

## Mode 4 -- Save Preset Confirmation

Shown when pressing Enter (short) in PLAY mode with unsaved changes.

```
Col: 0123456789012345
L0:  Save preset 0 ?
L1:    >No        Yes
```

### Controls

| Input | Action |
|-------|--------|
| Left / Right | Move between No and Yes |
| Enter (short) | Confirm selection |

On Yes: writes current parameters to the preset slot, saves to EEPROM,
and shows a 2-second completion message:

```
L0:  Preset 0 saved!
L1:  Write completed.
```

---

## Mode 5 -- Edit SSID

Character-by-character editor for WiFi SSID (up to 32 characters).
The hardware blink cursor marks the active character position.

```
Col: 0123456789012345
L0:  Edit SSID  1/32
L1:  MyNetworkName___
                ^blink
```

When the cursor moves past column 15, the text scrolls horizontally.

### Controls

| Input | Action |
|-------|--------|
| Left / Right | Move character position |
| Param encoder | Cycle through valid characters |
| Enter (short) | Confirm and go to SSID save dialog |

Valid SSID characters: space, 0-9, a-z, A-Z, `_`, `-`

---

## Mode 6 -- Edit PASS

Same interface as Edit SSID but for the WiFi passphrase (up to 63 characters).

```
Col: 0123456789012345
L0:  Edit PASS  1/63
L1:  mypassphrase____
                ^blink
```

Valid passphrase characters: space, 0-9, a-z

---

## Mode 7 -- Confirm SSID Change

```
Col: 0123456789012345
L0:  Apply new SSID?
L1:    >No        Yes
```

On Yes: saves the edited SSID to EEPROM and shows confirmation for 2 seconds.

---

## Mode 8 -- Confirm PASS Change

```
Col: 0123456789012345
L0:  Apply new PASS?
L1:    >No        Yes
```

On Yes: saves the edited passphrase to EEPROM and shows confirmation for 2 seconds.

---

## Mode 9 -- Reboot Confirmation

Shown when WiFi enable/disable is changed in SETTINGS.

```
Col: 0123456789012345
L0:  Reboot device?
L1:    >No        Yes
```

On Yes: calls `ESP.restart()`.

---

## Mode 10 -- WiFi Connected

Shown after successful WiFi connection at startup.
Allows the user to stay connected or reboot with WiFi disabled.

```
Col: 0123456789012345
L0:  WiFi connected!
L1:   >Stay   Reboot
```

| Choice | Action |
|--------|--------|
| Stay   | Remain in this mode (WiFi stays active) |
| Reboot | Disable WiFi, save to EEPROM, reboot |

---

## Startup Sequence

The LCD shows the following screens during boot:

1. **Splash** (2 seconds):
   ```
   L0:  BruesBreaker 1.1
   L1:  Starting up...
   ```
   Characters on L1 appear one-by-one (typewriter animation).

2. **EEPROM load error** (3 seconds, only if signature mismatch):
   ```
   L0:  EEPROM Load ERR
   L1:  Wrong signature
   ```

3. **WiFi connecting** (repeated until success or timeout):
   ```
   L0:  Connecting Wifi.
   L1:  Retry:3/10
   ```

4. **WiFi success** (2 seconds):
   ```
   L0:  http://bbp.local
   L1:  192.168.1.42
   ```

5. **WiFi failure** (4 seconds, then reboot):
   ```
   L0:  Wifi unavailable
   L1:  Wrong SSID/PASS
   ```

6. **WiFi disabled** (2 seconds):
   ```
   L0:  Wifi disabled
   L1:  See SET menu
   ```

---

## Mode Transition Diagram

```
                   Enter(long)
  PLAY ──────────────────────> MENU
   ^                            |
   |  Enter(short)              | select
   |  (if modified)             v
   |                     ┌─────────────┐
   +<── No ──< SAVE? >  │  PLAY       │
   |         (preset)    │  SET ───> SETTINGS ──> PLAY
   |                     │  INFO ──> WiFi INFO ──> PLAY
   |                     │  SSID ──> Edit SSID ──> Confirm SSID ──> PLAY
   |                     │  PASS ──> Edit PASS ──> Confirm PASS ──> PLAY
   |                     └─────────────┘
   |
   +<── SETTINGS ──(WiFi changed)──> Reboot? ──(Yes)──> ESP.restart()
```
