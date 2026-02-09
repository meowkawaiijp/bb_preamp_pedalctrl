// LCD(AQM1602Y, compatible with ST7032) handler
//
// 16x2 character LCD display handler for all application modes.
// Each mode renders a full-screen layout using lcd.clear() + redraw approach.

// ── Helper: Print "No" / "Yes" choice bar on line 1 ──
static void LcdPrintConfirmBar(void)
{
    lcd.setCursor(3, 1);
    lcd.print("No");
    lcd.setCursor(11, 1);
    lcd.print("Yes");

    switch(appVars.selected_yesno){
        case CONFIRM_NO:
            lcd.setCursor(2, 1);
            break;
        case CONFIRM_YES:
            lcd.setCursor(10, 1);
            break;
        default:
            break;
    }
    lcd.print(">");
}

// ── Main display update (called on every UI / MIDI / WiFi change) ──
void UpdateEntireLcdDisplay(void){
    // Erase entire display
    lcd.clear();

    // ─── Mode 0: PLAY ───
    if(appVars.mode == MODE_PLAY){
        if(appVars.selected_knob >= NUM_OF_KNOB) appVars.selected_knob = NUM_OF_KNOB - 1;

        lcd.noBlink();

        // Header labels (16 chars)
        //  Col: 0123456789012345
        //       VOL GAIN TRE BAS
        lcd.setCursor(0, 0);
        lcd.print("VOL GAIN TRE BAS");

        // Parameter values
        PrintInt2Digit(appParam.volume, 1, 1);   // cols 1-2
        PrintInt2Digit(appParam.gain, 5, 1);     // cols 5-6
        PrintInt3Digit(appParam.treble, 9, 1);   // cols 9-11
        PrintInt3Digit(appParam.bass, 13, 1);    // cols 13-15

        // Modified indicator: show '*' between gain and treble
        // when current preset has unsaved changes
        if(appVars.preset_changed == true){
            lcd.setCursor(7, 1);
            lcd.print("*");
        }

        // Selection cursor '>'
        switch(appVars.selected_knob){
            case KNOB_VOLUME:
                lcd.setCursor(0, 1);
                break;
            case KNOB_GAIN:
                lcd.setCursor(4, 1);
                break;
            case KNOB_TREBLE:
                lcd.setCursor(8, 1);
                break;
            case KNOB_BASS:
                lcd.setCursor(12, 1);
                break;
            default:
                break;
        }
        lcd.print(">");
    }

    // ─── Mode 1: SETTINGS ───
    else if(appVars.mode == MODE_SETTINGS){
        if(appVars.selected_settings >= NUM_OF_SETTINGS) appVars.selected_settings = NUM_OF_SETTINGS - 1;

        lcd.noBlink();

        // Header and labels (16 chars per line)
        //  Col: 0123456789012345
        //  L0:  SET MIDI:01 MX:9
        //  L1:   WiFi:On FSW:Pst
        lcd.setCursor(0, 0);
        lcd.print("SET");
        lcd.setCursor(4, 0);
        lcd.print("MIDI:");
        lcd.setCursor(12, 0);
        lcd.print("MX:");

        lcd.setCursor(1, 1);
        lcd.print("WiFi:");
        lcd.setCursor(9, 1);
        lcd.print("FSW:");

        // MIDI channel value (cols 9-10, line 0)
        PrintInt2Digit(appParam.midi_ch, 9, 0);

        // Preset max value (col 15, line 0)
        lcd.setCursor(15, 0);
        lcd.print(appParam.preset_max);

        // WiFi On/Off (cols 6-8, line 1)
        lcd.setCursor(6, 1);
        if(appParam.wifi_enable){
            lcd.print("On ");
        } else {
            lcd.print("Off");
        }

        // Foot switch mode (cols 13-15, line 1)
        lcd.setCursor(13, 1);
        switch(appParam.footsw_mode){
            case FOOTSW_PRESET:
                lcd.print("Pst");
                break;
            case FOOTSW_ALTERNATE:
                lcd.print("Alt");
                break;
            default:
                break;
        }

        // Selection cursor '>'
        switch(appVars.selected_settings){
            case SETTING_MIDI_CH:
                lcd.setCursor(3, 0);
                break;
            case SETTING_PRESET_MAX:
                lcd.setCursor(11, 0);
                break;
            case SETTING_WIFI_ONOFF:
                lcd.setCursor(0, 1);
                break;
            case SETTING_FOOTSW:
                lcd.setCursor(8, 1);
                break;
            default:
                break;
        }
        lcd.print(">");
    }

    // ─── Mode 2: MODE CHANGE (Menu) ───
    else if(appVars.mode == MODE_CHANGEMODE){
        if(appVars.selected_mode >= NUM_OF_CHANGEMODE) appVars.selected_mode = NUM_OF_CHANGEMODE - 1;

        lcd.noBlink();

        // Menu labels (16 chars per line)
        //  Col: 0123456789012345
        //  L0:  MENU: PLAY  SET
        //  L1:   INFO SSID  PASS
        lcd.setCursor(0, 0);
        lcd.print("MENU: PLAY  SET");
        lcd.setCursor(0, 1);
        lcd.print(" INFO SSID  PASS");

        // Selection cursor '>'
        switch(appVars.selected_mode){
            case MODECHANGE_PLAY:
                lcd.setCursor(6, 0);
                break;
            case MODECHANGE_SETTING:
                lcd.setCursor(12, 0);
                break;
            case MODECHANGE_WIFI:
                lcd.setCursor(0, 1);
                break;
            case MODECHANGE_SSID:
                lcd.setCursor(5, 1);
                break;
            case MODECHANGE_PASS:
                lcd.setCursor(11, 1);
                break;
            default:
                break;
        }
        lcd.print(">");
    }

    // ─── Mode 3: WiFi INFO ───
    else if(appVars.mode == MODE_SHOW_URI){
        lcd.noBlink();

        lcd.setCursor(0, 0);
        lcd.print("http://");
        lcd.print(MDNS_STR);
        lcd.print(".local");

        lcd.setCursor(0, 1);
        if(appVars.available_wifi == true){
            lcd.print(WiFi.localIP());
        } else {
            lcd.print("(No connection)");
        }
    }

    // ─── Mode 4-8: Confirmation dialogs ───
    else if(appVars.mode == MODE_CONFIRM_WRITE_PRESET
         || appVars.mode == MODE_CONFIRM_SSID
         || appVars.mode == MODE_CONFIRM_PASS
         || appVars.mode == MODE_CONFIRM_RESET){
        if(appVars.selected_yesno >= NUM_OF_CONFIRM) appVars.selected_yesno = NUM_OF_CONFIRM - 1;

        lcd.noBlink();

        // Question line (line 0)
        lcd.setCursor(0, 0);
        if(appVars.mode == MODE_CONFIRM_WRITE_PRESET){
            lcd.print("Save preset ");
            lcd.print(appParam.presetNo);
            lcd.print(" ?");
        }
        else if(appVars.mode == MODE_CONFIRM_SSID){
            lcd.print("Apply new SSID?");
        }
        else if(appVars.mode == MODE_CONFIRM_PASS){
            lcd.print("Apply new PASS?");
        }
        else if(appVars.mode == MODE_CONFIRM_RESET){
            lcd.print("Reboot device?");
        }

        // No / Yes bar with cursor (line 1)
        LcdPrintConfirmBar();
    }

    // ─── WiFi connecting mode ───
    else if(appVars.mode == MODE_WIFI_CONNECTING){
        if(appVars.selected_yesno >= NUM_OF_CONFIRM) appVars.selected_yesno = NUM_OF_CONFIRM - 1;

        lcd.noBlink();
        lcd.setCursor(0, 0);
        lcd.print("WiFi connected!");

        lcd.setCursor(2, 1);
        lcd.print("Stay");
        lcd.setCursor(9, 1);
        lcd.print("Reboot");

        // Selection cursor '>'
        switch(appVars.selected_yesno){
            case CONFIRM_NO:
                lcd.setCursor(1, 1);
                break;
            case CONFIRM_YES:
                lcd.setCursor(8, 1);
                break;
            default:
                break;
        }
        lcd.print(">");
    }

    // ─── Edit SSID ───
    else if(appVars.mode == MODE_EDIT_SSID){
        char tmpstr[17];

        lcd.blink();
        lcd.setCursor(0, 0);
        lcd.print("Edit SSID ");
        sprintf(tmpstr, "%2d/%2d", appVars.ssid_currectPos + 1, WIFI_SSID_MAX_LEN);
        lcd.setCursor(11, 0);
        lcd.print(tmpstr);

        // Scrolling display window
        int cursor_disp_pos, disp_start_pos;
        if(appVars.ssid_currectPos < 16){
            cursor_disp_pos = appVars.ssid_currectPos;
            disp_start_pos = 0;
        } else {
            cursor_disp_pos = 15;
            disp_start_pos = appVars.ssid_currectPos - 15;
        }
        char tmpDispSsid[33];
        strcpy(tmpDispSsid, &(appVars.edited_wifi_ssid[disp_start_pos]));
        if(strlen(tmpDispSsid) > 16) tmpDispSsid[16] = '\0';
        lcd.setCursor(0, 1);
        lcd.print(tmpDispSsid);
        lcd.setCursor(cursor_disp_pos, 1);
    }

    // ─── Edit PASSPHRASE ───
    else if(appVars.mode == MODE_EDIT_PASS){
        char tmpstr[17];

        lcd.blink();
        lcd.setCursor(0, 0);
        lcd.print("Edit PASS ");
        sprintf(tmpstr, "%2d/%2d", appVars.pass_currectPos + 1, WIFI_PASS_MAX_LEN);
        lcd.setCursor(11, 0);
        lcd.print(tmpstr);

        // Scrolling display window
        int cursor_disp_pos, disp_start_pos;
        if(appVars.pass_currectPos < 16){
            cursor_disp_pos = appVars.pass_currectPos;
            disp_start_pos = 0;
        } else {
            cursor_disp_pos = 15;
            disp_start_pos = appVars.pass_currectPos - 15;
        }
        char tmpDispPass[64];
        strcpy(tmpDispPass, &(appVars.edited_wifi_pass[disp_start_pos]));
        if(strlen(tmpDispPass) > 16) tmpDispPass[16] = '\0';
        lcd.setCursor(0, 1);
        lcd.print(tmpDispPass);
        lcd.setCursor(cursor_disp_pos, 1);
    }
}
