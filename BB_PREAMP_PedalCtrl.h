// BB_PREAMP_PedalCtrl.h
// Main configuration and type definitions for the BB-Preamp Pedal Controller.
//
// Target: ESP32-C3 (Seeed XIAO ESP32C3)
// LCD:    AQM1602Y (ST7032 compatible, 16x2)

#pragma once

// ── Time zone ──
#define JST     3600*9

// ── WiFi default credentials (overwritten by EEPROM) ──
#define STASSID "ssid"
#define STAPSK  "passphrase"
#define WIFI_CONNECTION_RETRY 10

// ── mDNS hostname ──
#define MDNS_STR "bbp"

// ── EEPROM signature (change to invalidate stored data) ──
#define SIGNATURE "BBPRE100"

// ── Digital pin assignments ──
#define LED1_PIN D3
#define SW1 D2
#define SW2 D3

// ── Hardware timer (1 kHz interrupt for input polling) ──
#define PRESCALER_DIVIDE_RATE 80    // Divide 80 MHz APB to 1 MHz
#define TIMER_UI_INTERVAL 1000      // Callback period in microseconds

// ── UI button indices (into sdi.sw[]) ──
#define UI_BUTTON_LEFT  0
#define UI_BUTTON_RIGHT 1
#define UI_BUTTON_ENTER 2
#define UI_BUTTON_FOOT  3

// ── Encoder indices (into sdi.encoder[]) ──
#define UI_ENCODER_PARAM  0   // Parameter value encoder
#define UI_ENCODER_PRESET 1   // Preset number encoder

// ═════════════════════════════════════════════
//  Application modes (appVars.mode)
// ═════════════════════════════════════════════
#define NUM_OF_MODE 11
#define MODE_PLAY                  0   // Normal play screen
#define MODE_SETTINGS              1   // System settings (MIDI ch, preset max, WiFi, footsw)
#define MODE_CHANGEMODE            2   // Mode selection menu
#define MODE_SHOW_URI              3   // Show WiFi connection info
#define MODE_CONFIRM_WRITE_PRESET  4   // Confirm preset save
#define MODE_EDIT_SSID             5   // Edit WiFi SSID character-by-character
#define MODE_EDIT_PASS             6   // Edit WiFi passphrase character-by-character
#define MODE_CONFIRM_SSID          7   // Confirm SSID change
#define MODE_CONFIRM_PASS          8   // Confirm passphrase change
#define MODE_CONFIRM_RESET         9   // Confirm reboot
#define MODE_WIFI_CONNECTING       10  // WiFi connected, stay or reboot

// ── Knob selection in PLAY mode ──
#define NUM_OF_KNOB 4
#define KNOB_VOLUME 0
#define KNOB_GAIN   1
#define KNOB_TREBLE 2
#define KNOB_BASS   3

// ── Setting selection in SETTINGS mode ──
#define NUM_OF_SETTINGS 4
#define SETTING_MIDI_CH    0
#define SETTING_PRESET_MAX 1
#define SETTING_WIFI_ONOFF 2
#define SETTING_FOOTSW     3

// ── Menu items in MODE CHANGE screen ──
#define NUM_OF_CHANGEMODE 5
#define MODECHANGE_PLAY    0
#define MODECHANGE_SETTING 1
#define MODECHANGE_WIFI    2   // Show connection info
#define MODECHANGE_SSID    3   // Edit SSID
#define MODECHANGE_PASS    4   // Edit passphrase

// ── Confirmation dialog choices ──
#define NUM_OF_CONFIRM 2
#define CONFIRM_NO  0
#define CONFIRM_YES 1

// ── Foot switch behaviour ──
#define FOOTSW_ALTERNATE 0   // Toggle effect on/off
#define FOOTSW_PRESET    1   // Cycle through presets

// ── UI change status codes (returned by UiEncoderHandler) ──
#define UI_CHANGE_PRESET         11
#define UI_CHANGE_PLAY_SELECTION 12
#define UI_CHANGE_PLAY_EFFECT    13
#define UI_CHANGE_PLAY_FEEDBACK  14
#define UI_CHANGE_PLAY_TIME      15
#define UI_CHANGE_SET_SELECTION  16
#define UI_CHANGE_SET_MIDI       17
#define UI_CHANGE_SET_FOOTSW    18
#define UI_CHANGE_MODE_SELECTION 19

// ── Delay time range (reserved for future use) ──
#define DELAY_TIME_MAX 580
#define DELAY_TIME_MIN 50
#define UI_DELAYTIME_STEP 5

// ═════════════════════════════════════════════
//  WiFi SSID / Passphrase
// ═════════════════════════════════════════════
#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 63

// Valid characters for WiFi credential editing
#define VALID_SSID " 0123456789abcdefghijklnmopqrstuvwxyzABCDEFGHIJKLNMOPQRSTUVWZYZ_-"
#define VALID_PASS " 0123456789abcdefghijklnmopqrstuvwxyz"

// ── Debug ──
#define LEN_DEBUG_MESSAGE 1024

// ═════════════════════════════════════════════
//  Data structures
// ═════════════════════════════════════════════

// Parameters saved to EEPROM
typedef struct {
  int onoff;                            // Effect relay: 0=off, 1=on
  int volume;                           // Volume (0-99)
  int gain;                             // Gain   (0-99)
  int treble;                           // Treble (-50 to +50)
  int bass;                             // Bass   (-50 to +50)
  int presetNo;                         // Active preset (0-9)
  int midi_ch;                          // MIDI channel (1-16)
  int footsw_mode;                      // 0=alternate, 1=preset cycle
  char wifi_ssid[WIFI_SSID_MAX_LEN+1]; // WiFi SSID (max 32 chars)
  char wifi_pass[WIFI_PASS_MAX_LEN+1]; // WiFi passphrase (max 63 chars)
  int wifi_enable;                      // 0=disabled, 1=enabled
  int preset_max;                       // Highest preset selectable by foot SW (2-9)
} Type_AppParams;

// Runtime variables (not persisted)
typedef struct {
  int selected_knob;       // Currently selected knob in PLAY mode
  int selected_settings;   // Currently selected item in SETTINGS mode
  int selected_mode;       // Currently selected item in MODE CHANGE menu
  int selected_yesno;      // Currently selected choice in confirmation dialogs
  int preset_changed;      // True when parameters differ from stored preset
  int available_wifi;      // True if WiFi connection succeeded
  int mode;                // Current application mode (MODE_*)
  int changed_via_wifi;    // Set when preset data updated via WebUI
  int changed_wifi_7seg;   // Set when 7-seg animation tick fires
  int ssid_currectPos;     // Cursor position in SSID editor
  int pass_currectPos;     // Cursor position in PASS editor
  char edited_wifi_ssid[WIFI_SSID_MAX_LEN+1]; // SSID edit buffer
  char edited_wifi_pass[WIFI_PASS_MAX_LEN+1]; // PASS edit buffer
  int edit_cursor_char_id; // Index into valid character table
  int wifi_onoff_changed;  // True when WiFi enable toggled (needs reboot)
} Type_AppInternalVars;

// Per-preset tone parameters
#define NUM_PRESET 10
typedef struct {
  int volume;
  int gain;
  int treble;
  int bass;
} Type_PresetParams;

// EEPROM layout
typedef struct {
  char signature[10];
  Type_AppParams params;
  Type_PresetParams presets[NUM_PRESET];
} EEPROM_DATA;
