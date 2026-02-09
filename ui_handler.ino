// UI (tact SWs, rotary encoders) handler

#include "BB_PREAMP_PedalCtrl.h"
#include "myUtil.h"
#include "mySPI.h"
#include "web_server.h"
#include "ui_handler.h"

// ── Helper: find character index in a validation table ──
static int GetCharIndexInTable(const char *valid_chars, char cursorChar)
{
  int len = strlen(valid_chars);
  for(int i = 0; i < len; i++){
    if(valid_chars[i] == cursorChar){
      return i;
    }
  }
  return 0;
}

// ── Helper: commit edited WiFi text (trim trailing spaces) ──
static void CommitEditedWifiText(char *dest, char *edited, int max_len)
{
  for(int i = 0; i < max_len; i++){
    if(edited[i] == ' '){
      edited[i] = '\0';
      break;
    }
  }
  edited[max_len] = '\0';
  strcpy(dest, edited);
}

// ── Helper: handle Left/Right buttons for Yes/No selection ──
// Returns true if selection changed.
static bool HandleConfirmLeftRight(void)
{
  bool changed = false;
  if(sdi.sw[UI_BUTTON_LEFT].trigger_shortpress == true){
    sdi.sw[UI_BUTTON_LEFT].trigger_shortpress = false;
    if(appVars.selected_yesno > 0){
      appVars.selected_yesno--;
      changed = true;
    }
  }
  if(sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress == true){
    sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress = false;
    if(appVars.selected_yesno < NUM_OF_CONFIRM - 1){
      appVars.selected_yesno++;
      changed = true;
    }
  }
  return changed;
}

// ── Helper: consume long-press enter (no-op in most modes) ──
static void ConsumeEnterLongPress(void)
{
  if(sdi.sw[UI_BUTTON_ENTER].trigger_longpress == true){
    sdi.sw[UI_BUTTON_ENTER].trigger_longpress = false;
  }
}

// ── Helper: read encoder with proper reset ──
// Returns +1 for CW, -1 for CCW, 0 for no movement.
static int ReadEncoderWithReset(int encoder_index)
{
  int dir = sdi.encoder[encoder_index].rotation_dir;
  if(dir != ROTATION_STOP){
    sdi.encoder[encoder_index].rotation_dir = ROTATION_STOP;
  }
  return dir;
}

// Get current character index in SSID validation table
static int GetCharCurrentNumber_SSID()
{
  char cursorChar = appVars.edited_wifi_ssid[appVars.ssid_currectPos];
  return GetCharIndexInTable(valid_ssid_char, cursorChar);
}

// Get current character index in PASS validation table
static int GetCharCurrentNumber_PASS()
{
  char cursorChar = appVars.edited_wifi_pass[appVars.pass_currectPos];
  return GetCharIndexInTable(valid_pass_char, cursorChar);
}


// ═══════════════════════════════════════════════════════
//  Button Handler (called from main loop)
// ═══════════════════════════════════════════════════════
int UiButtonHandler(void){
  int ret = false;

  // ── PLAY mode ──
  if(appVars.mode == MODE_PLAY){
    // Left button: move selection left
    if(sdi.sw[UI_BUTTON_LEFT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_LEFT].trigger_shortpress = false;
      if(appVars.selected_knob > 0){
        appVars.selected_knob--;
        ret = true;
      }
    }
    // Right button: move selection right
    if(sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress = false;
      if(appVars.selected_knob < NUM_OF_KNOB - 1){
        appVars.selected_knob++;
        ret = true;
      }
    }
    // Enter short-press: save preset (if changed)
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appVars.preset_changed == true){
        appVars.mode = MODE_CONFIRM_WRITE_PRESET;
        appVars.selected_yesno = 0;
      }
      ret = true;
    }
    // Enter long-press: open mode change menu
    if(sdi.sw[UI_BUTTON_ENTER].trigger_longpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_longpress = false;
      appVars.mode = MODE_CHANGEMODE;
      ret = true;
    }
    // Foot switch
    if(sdi.sw[UI_BUTTON_FOOT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_FOOT].trigger_shortpress = false;
      if(appParam.footsw_mode == FOOTSW_PRESET){
        appParam.presetNo++;
        if(appParam.presetNo > appParam.preset_max) appParam.presetNo = 0;
        LoadPresetToCurrentParam(appParam.presetNo);
      }
      if(appParam.footsw_mode == FOOTSW_ALTERNATE){
        appParam.onoff = 1 - appParam.onoff;
      }
      ret = true;
    }
  }

  // ── SETTINGS mode ──
  else if(appVars.mode == MODE_SETTINGS){
    // Left button: move selection left
    if(sdi.sw[UI_BUTTON_LEFT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_LEFT].trigger_shortpress = false;
      if(appVars.selected_settings > 0){
        appVars.selected_settings--;
        ret = true;
      }
    }
    // Right button: move selection right
    if(sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress = false;
      if(appVars.selected_settings < NUM_OF_SETTINGS - 1){
        appVars.selected_settings++;
        ret = true;
      }
    }
    // Enter short-press: save settings and return to play
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appParam.footsw_mode == FOOTSW_PRESET) appParam.onoff = true;
      save_eeprom();
      if(appVars.wifi_onoff_changed == true){
        appVars.mode = MODE_CONFIRM_RESET;
        appVars.wifi_onoff_changed = false;
      } else {
        appVars.mode = MODE_PLAY;
      }
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── MODE CHANGE (menu) ──
  else if(appVars.mode == MODE_CHANGEMODE){
    // Left button
    if(sdi.sw[UI_BUTTON_LEFT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_LEFT].trigger_shortpress = false;
      if(appVars.selected_mode > 0){
        appVars.selected_mode--;
        ret = true;
      }
    }
    // Right button
    if(sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress = false;
      if(appVars.selected_mode < NUM_OF_CHANGEMODE - 1){
        appVars.selected_mode++;
        ret = true;
      }
    }
    // Enter short-press: select mode
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appVars.selected_mode == MODECHANGE_PLAY){
        appVars.mode = MODE_PLAY;
      }
      else if(appVars.selected_mode == MODECHANGE_SETTING){
        appVars.mode = MODE_SETTINGS;
      }
      else if(appVars.selected_mode == MODECHANGE_WIFI){
        appVars.mode = MODE_SHOW_URI;
      }
      else if(appVars.selected_mode == MODECHANGE_SSID){
        appVars.mode = MODE_EDIT_SSID;
        appVars.ssid_currectPos = 0;
        // Initialize edit buffer with current SSID
        memset(appVars.edited_wifi_ssid, ' ', WIFI_SSID_MAX_LEN);
        appVars.edited_wifi_ssid[WIFI_SSID_MAX_LEN] = '\0';
        memcpy(appVars.edited_wifi_ssid, appParam.wifi_ssid, strlen(appParam.wifi_ssid));
        appVars.edit_cursor_char_id = GetCharCurrentNumber_SSID();
      }
      else { // MODECHANGE_PASS
        appVars.mode = MODE_EDIT_PASS;
        appVars.pass_currectPos = 0;
        // Initialize edit buffer with current PASS
        memset(appVars.edited_wifi_pass, ' ', WIFI_PASS_MAX_LEN);
        appVars.edited_wifi_pass[WIFI_PASS_MAX_LEN] = '\0';
        memcpy(appVars.edited_wifi_pass, appParam.wifi_pass, strlen(appParam.wifi_pass));
        appVars.edit_cursor_char_id = GetCharCurrentNumber_PASS();
      }
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── SHOW URI (WiFi info) ──
  else if(appVars.mode == MODE_SHOW_URI){
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      appVars.mode = MODE_PLAY;
      if(appParam.footsw_mode == FOOTSW_PRESET) appParam.onoff = true;
      ret = true;
    }
  }

  // ── Preset write confirmation ──
  else if(appVars.mode == MODE_CONFIRM_WRITE_PRESET){
    if(HandleConfirmLeftRight()) ret = true;
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appVars.selected_yesno == CONFIRM_NO){
        appVars.mode = MODE_PLAY;
      } else {
        // Write current params to preset memory
        presetParams[appParam.presetNo].volume = appParam.volume;
        presetParams[appParam.presetNo].gain = appParam.gain;
        presetParams[appParam.presetNo].treble = appParam.treble;
        presetParams[appParam.presetNo].bass = appParam.bass;
        appVars.preset_changed = false;
        save_eeprom();
        // Show completion message
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Preset ");
        lcd.print(appParam.presetNo);
        lcd.print(" saved!");
        lcd.setCursor(0, 1);
        lcd.print("Write completed.");
        delay(2000);
        appVars.mode = MODE_PLAY;
      }
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── Reboot confirmation ──
  else if(appVars.mode == MODE_CONFIRM_RESET){
    if(HandleConfirmLeftRight()) ret = true;
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appVars.selected_yesno == CONFIRM_NO){
        appVars.mode = MODE_PLAY;
      } else {
        ESP.restart();
      }
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── WiFi connecting ──
  else if(appVars.mode == MODE_WIFI_CONNECTING){
    if(HandleConfirmLeftRight()) ret = true;
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appVars.selected_yesno == CONFIRM_NO){
        // Stay in WiFi connected mode
        appVars.mode = MODE_WIFI_CONNECTING;
      } else {
        // Disable WiFi and reboot
        appParam.wifi_enable = false;
        save_eeprom();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi disabled.");
        lcd.setCursor(0, 1);
        lcd.print("Rebooting..");
        delay(2000);
        ESP.restart();
      }
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── SSID write confirmation ──
  else if(appVars.mode == MODE_CONFIRM_SSID){
    if(HandleConfirmLeftRight()) ret = true;
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appVars.selected_yesno == CONFIRM_NO){
        appVars.mode = MODE_PLAY;
      } else {
        // Commit edited SSID
        CommitEditedWifiText(appParam.wifi_ssid, appVars.edited_wifi_ssid, WIFI_SSID_MAX_LEN);
        if(debug_output_to_html && strlen(debug_message) < LEN_DEBUG_MESSAGE - 100){
          char text[100];
          sprintf(text, "SSID: [%s]<br />", appParam.wifi_ssid);
          strcat(debug_message, text);
        }
        save_eeprom();
        // Show completion message
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SSID saved.");
        lcd.setCursor(0, 1);
        lcd.print(appParam.wifi_ssid);
        delay(2000);
        appVars.mode = MODE_PLAY;
      }
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── PASS write confirmation ──
  else if(appVars.mode == MODE_CONFIRM_PASS){
    if(HandleConfirmLeftRight()) ret = true;
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      if(appVars.selected_yesno == CONFIRM_NO){
        appVars.mode = MODE_PLAY;
      } else {
        // Commit edited passphrase
        CommitEditedWifiText(appParam.wifi_pass, appVars.edited_wifi_pass, WIFI_PASS_MAX_LEN);
        if(debug_output_to_html && strlen(debug_message) < LEN_DEBUG_MESSAGE - 100){
          char text[100];
          sprintf(text, "PASS: [%s]<br />", appParam.wifi_pass);
          strcat(debug_message, text);
        }
        save_eeprom();
        // Show completion message
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PASS saved.");
        lcd.setCursor(0, 1);
        lcd.print(appParam.wifi_pass);
        delay(2000);
        appVars.mode = MODE_PLAY;
      }
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── Edit SSID ──
  else if(appVars.mode == MODE_EDIT_SSID){
    // Left: move cursor left
    if(sdi.sw[UI_BUTTON_LEFT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_LEFT].trigger_shortpress = false;
      if(appVars.ssid_currectPos > 0){
        appVars.ssid_currectPos--;
        appVars.edit_cursor_char_id = GetCharCurrentNumber_SSID();
        ret = true;
      }
    }
    // Right: move cursor right
    if(sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress = false;
      if(appVars.ssid_currectPos < WIFI_SSID_MAX_LEN - 1){
        appVars.ssid_currectPos++;
        appVars.edit_cursor_char_id = GetCharCurrentNumber_SSID();
        ret = true;
      }
    }
    // Enter: confirm edit
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      appVars.mode = MODE_CONFIRM_SSID;
      appVars.selected_yesno = 0;
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  // ── Edit PASS ──
  else if(appVars.mode == MODE_EDIT_PASS){
    // Left: move cursor left
    if(sdi.sw[UI_BUTTON_LEFT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_LEFT].trigger_shortpress = false;
      if(appVars.pass_currectPos > 0){
        appVars.pass_currectPos--;
        appVars.edit_cursor_char_id = GetCharCurrentNumber_PASS();
        ret = true;
      }
    }
    // Right: move cursor right
    if(sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_RIGHT].trigger_shortpress = false;
      if(appVars.pass_currectPos < WIFI_PASS_MAX_LEN - 1){
        appVars.pass_currectPos++;
        appVars.edit_cursor_char_id = GetCharCurrentNumber_PASS();
        ret = true;
      }
    }
    // Enter: confirm edit
    if(sdi.sw[UI_BUTTON_ENTER].trigger_shortpress == true){
      sdi.sw[UI_BUTTON_ENTER].trigger_shortpress = false;
      appVars.mode = MODE_CONFIRM_PASS;
      appVars.selected_yesno = 0;
      ret = true;
    }
    ConsumeEnterLongPress();
  }

  return ret;
}


// ═══════════════════════════════════════════════════════
//  Encoder Handler (called from main loop)
// ═══════════════════════════════════════════════════════
int UiEncoderHandler(void){
  int ret = false;

  // ── PLAY mode ──
  if(appVars.mode == MODE_PLAY){
    // Parameter encoder: adjust selected knob value
    int paramDir = ReadEncoderWithReset(UI_ENCODER_PARAM);
    if(paramDir != ROTATION_STOP){
      int delta = (paramDir == ROTATION_PLUS) ? 1 : -1;
      switch(appVars.selected_knob){
        case KNOB_VOLUME:
          if(StepClampedInt(&appParam.volume, delta, 0, 99)){
            appVars.preset_changed = true;
            ret = true;
          }
          break;
        case KNOB_GAIN:
          if(StepClampedInt(&appParam.gain, delta, 0, 99)){
            appVars.preset_changed = true;
            ret = true;
          }
          break;
        case KNOB_TREBLE:
          if(StepClampedInt(&appParam.treble, delta, -50, 50)){
            appVars.preset_changed = true;
            ret = true;
          }
          break;
        case KNOB_BASS:
          if(StepClampedInt(&appParam.bass, delta, -50, 50)){
            appVars.preset_changed = true;
            ret = true;
          }
          break;
        default:
          break;
      }
    }

    // Preset encoder: change preset number
    int presetDir = ReadEncoderWithReset(UI_ENCODER_PRESET);
    if(presetDir != ROTATION_STOP){
      int delta = (presetDir == ROTATION_PLUS) ? 1 : -1;
      if(StepClampedInt(&appParam.presetNo, delta, 0, NUM_PRESET - 1)){
        appVars.preset_changed = false;
        ret = UI_CHANGE_PRESET;
      }
    }

    // Load preset when changed
    if(ret == UI_CHANGE_PRESET){
      LoadPresetToCurrentParam(appParam.presetNo);
    }
  }

  // ── SETTINGS mode ──
  else if(appVars.mode == MODE_SETTINGS){
    int dir = ReadEncoderWithReset(UI_ENCODER_PARAM);
    if(dir != ROTATION_STOP){
      int delta = (dir == ROTATION_PLUS) ? 1 : -1;
      switch(appVars.selected_settings){
        case SETTING_MIDI_CH:
          if(StepClampedInt(&appParam.midi_ch, delta, 1, 16)){
            ret = true;
          }
          break;
        case SETTING_PRESET_MAX:
          if(StepClampedInt(&appParam.preset_max, delta, 2, NUM_PRESET - 1)){
            ret = true;
          }
          break;
        case SETTING_WIFI_ONOFF:
          if(StepClampedInt(&appParam.wifi_enable, delta, 0, 1)){
            appVars.wifi_onoff_changed = true;
            ret = true;
          }
          break;
        case SETTING_FOOTSW:
          if(StepClampedInt(&appParam.footsw_mode, delta, FOOTSW_ALTERNATE, FOOTSW_PRESET)){
            ret = true;
          }
          break;
        default:
          break;
      }
    }
  }

  // ── Edit SSID ──
  else if(appVars.mode == MODE_EDIT_SSID){
    int dir = ReadEncoderWithReset(UI_ENCODER_PARAM);
    if(dir != ROTATION_STOP){
      int validlen = strlen(VALID_SSID);
      if(dir == ROTATION_PLUS){
        appVars.edit_cursor_char_id++;
        if(appVars.edit_cursor_char_id >= validlen) appVars.edit_cursor_char_id = 0;
      } else {
        appVars.edit_cursor_char_id--;
        if(appVars.edit_cursor_char_id < 0) appVars.edit_cursor_char_id = validlen - 1;
      }
      appVars.edited_wifi_ssid[appVars.ssid_currectPos] = valid_ssid_char[appVars.edit_cursor_char_id];
      ret = true;
    }
  }

  // ── Edit PASS ──
  else if(appVars.mode == MODE_EDIT_PASS){
    int dir = ReadEncoderWithReset(UI_ENCODER_PARAM);
    if(dir != ROTATION_STOP){
      int validlen = strlen(VALID_PASS);
      if(dir == ROTATION_PLUS){
        appVars.edit_cursor_char_id++;
        if(appVars.edit_cursor_char_id >= validlen) appVars.edit_cursor_char_id = 0;
      } else {
        appVars.edit_cursor_char_id--;
        if(appVars.edit_cursor_char_id < 0) appVars.edit_cursor_char_id = validlen - 1;
      }
      appVars.edited_wifi_pass[appVars.pass_currectPos] = valid_pass_char[appVars.edit_cursor_char_id];
      ret = true;
    }
  }

  return ret;
}
