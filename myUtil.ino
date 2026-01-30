// Utility: string copy with length limit
void CopyCharByLength(char *org, char *dest, int max_length)
{
  int i=0;
  while(1)
  {
    if(org[i]=='\0') break;
    if(i>=max_length) break;
    dest[i] = org[i];
    i++;
  }
  dest[i] = '\0';
}

// Utility: get real time string HH:MM.SS
void getRTC( char* timestr )
{
  time_t t;
  struct tm *tm;
  t = time(NULL);
  tm = localtime(&t);
  sprintf(timestr, "%02d:%02d.%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);  
}

// Utility: print RTC on LCD
void LcdPrintRTC(void)
{
    // Real time
  char rtcstr[16];
  getRTC(rtcstr);
  lcd.setCursor(0, 1);
  lcd.print(rtcstr); 
}

// Utility: Erace LCD Chars (fill with space)
void EraceLCD(int posx, int posy, int numOfChar)
{
  int i;
  lcd.setCursor(posx, posy);
  for(i=0; i<numOfChar; i++)
  {
    lcd.print(" ");
  }
  lcd.setCursor(posx, posy);
}

// print 8bit binary to LCD
void PrintBinLCD(unsigned char bindata)
{
  int i;
  unsigned char tmp;
  tmp = bindata;
  for(i=0;i<8;i++){
    if(tmp&0x80) lcd.print("1");
    else lcd.print("0");
    tmp = tmp<<1;
  }
}

// print 2 digit integer with justifing to the right
void PrintInt2Digit(int value, int px, int py){
  int tmp;
  
  EraceLCD(px, py, 2);
  tmp=value;
  tmp%=100;
  if(tmp<10) lcd.print("0");
  lcd.print(tmp);
}

// print 3 digit integer with plus/minus display
void PrintInt3Digit(int value, int px, int py){
  int tmp;
  
  EraceLCD(px, py, 3);
  if(value<-9){
    lcd.print(value);
  }
  else if(value<0)
  {
    lcd.print("-0");
    lcd.print(value*-1);
  }
  else if(value==0)
  {
    lcd.print(" 00");
  }
  else if(value<10)
  {
    lcd.print("+0");
    lcd.print(value);
  }
  else //if(value<51)
  {
    lcd.print("+");
    lcd.print(value);
  }
}

// print 4 digit integer with justifing to the right
void PrintInt4Digit(int value, int px, int py){
  int tmp;
  
  EraceLCD(px, py, 4);
  tmp=value;
  tmp%=10000;
  if(tmp<1000) lcd.print("0");
  if(tmp<100) lcd.print("0");
  if(tmp<10) lcd.print("0");
  lcd.print(tmp);
}

bool UpdateClampedInt(int *target, int value, int minValue, int maxValue){
  if(value < minValue) value = minValue;
  if(value > maxValue) value = maxValue;
  if(*target == value){
    return false;
  }
  *target = value;
  return true;
}

bool StepClampedInt(int *target, int delta, int minValue, int maxValue){
  return UpdateClampedInt(target, *target + delta, minValue, maxValue);
}

// Range check for preset values
void CheckAndCorrectPreset(void){
  int i;

  // check preset value
  for(i=0; i<NUM_PRESET; i++)
  {
    UpdateClampedInt(&presetParams[i].volume, presetParams[i].volume, 0, 99);
    UpdateClampedInt(&presetParams[i].gain, presetParams[i].gain, 0, 99);
    UpdateClampedInt(&presetParams[i].treble, presetParams[i].treble, -50, 50);
    UpdateClampedInt(&presetParams[i].bass, presetParams[i].bass, -50, 50);
  }  
}

// Range check for app params loaded from EEPROM
static void CheckAndCorrectAppParams(void){
  if(appParam.onoff < 0) appParam.onoff = 0;
  if(appParam.onoff > 1) appParam.onoff = 1;
  if(appParam.volume < 0) appParam.volume = 0;
  if(appParam.volume > 99) appParam.volume = 99;
  if(appParam.gain < 0) appParam.gain = 0;
  if(appParam.gain > 99) appParam.gain = 99;
  if(appParam.treble < -50) appParam.treble = -50;
  if(appParam.treble > 50) appParam.treble = 50;
  if(appParam.bass < -50) appParam.bass = -50;
  if(appParam.bass > 50) appParam.bass = 50;
  if(appParam.presetNo < 0) appParam.presetNo = 0;
  if(appParam.presetNo >= NUM_PRESET) appParam.presetNo = NUM_PRESET - 1;
  if(appParam.midi_ch < 1) appParam.midi_ch = 1;
  if(appParam.midi_ch > 16) appParam.midi_ch = 16;
  if(appParam.footsw_mode < FOOTSW_ALTERNATE) appParam.footsw_mode = FOOTSW_ALTERNATE;
  if(appParam.footsw_mode > FOOTSW_PRESET) appParam.footsw_mode = FOOTSW_PRESET;
  if(appParam.wifi_enable < 0) appParam.wifi_enable = 0;
  if(appParam.wifi_enable > 1) appParam.wifi_enable = 1;
  if(appParam.preset_max < 2) appParam.preset_max = 2;
  if(appParam.preset_max > (NUM_PRESET - 1)) appParam.preset_max = NUM_PRESET - 1;
  appParam.wifi_ssid[WIFI_SSID_MAX_LEN] = '\0';
  appParam.wifi_pass[WIFI_PASS_MAX_LEN] = '\0';
}

// Read from EEPROM
int load_eeprom() {
    EEPROM.get<EEPROM_DATA>(0, eeprom);
    if(strcmp(eeprom.signature, SIGNATURE)==0)
    {
        memcpy( &appParam, &eeprom.params, sizeof(Type_AppParams) );
        memcpy( &presetParams[0], &eeprom.presets[0], sizeof(Type_PresetParams)*NUM_PRESET );    

        CheckAndCorrectAppParams();
        // verify preset value
        CheckAndCorrectPreset();
        return true;
    }
    else
    { 
      // Erase buffer memory if signature is wrong
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("EEPROM Load ERR");
      lcd.setCursor(0,1);
      lcd.print("Wrong signature");
      //lcd.setCursor(0,1);
      //lcd.print(eeprom.signature);
      delay(3000);
      return false;
    }
}

// Save to EEPROM
void save_eeprom() {
    //EEPROMに設定を保存する。
    memset(&eeprom, 0, sizeof(EEPROM_DATA));
    strcpy(eeprom.signature, SIGNATURE);
    memcpy( &eeprom.params, &appParam, sizeof(Type_AppParams) );
    memcpy( &eeprom.presets[0], &presetParams[0], sizeof(Type_PresetParams)*NUM_PRESET );    
    EEPROM.put<EEPROM_DATA>(0, eeprom);
    EEPROM.commit(); //必須
}

// AD5263 pin assign
#define AD5263_CLK D8
#define AD5263_SDI D10
#define AD5263_CS  D9

void SetupAD5263(void){
  // def
  pinMode(AD5263_CLK, OUTPUT);
  pinMode(AD5263_SDI, OUTPUT);
  pinMode(AD5263_CS, OUTPUT);
  // initial state
  digitalWrite(AD5263_CLK, HIGH);
  digitalWrite(AD5263_SDI, HIGH);
  digitalWrite(AD5263_CS, HIGH);
}

// Send serial data to HC595 and AD5263_ADDR
//   HC595 : external 8bit shift register (0-0x0f) connected to AD5263 SDO, 8bit
//   AD5263_ADDR : ch of AD5263 (0-3), 2bit
//   AD5263_DATA : step value of AD5263 (0-255), 8bit
void MySpiTransfer(int SEG7, int HC595, int AD5263_ADDR, int AD5263_DATA){
  int i;
  int shifted_data=0;
  // initial
  digitalWrite(AD5263_CLK, HIGH);
  digitalWrite(AD5263_SDI, HIGH);
  digitalWrite(AD5263_CS, HIGH); 

  // prepare to latch ( change CS to LOW )
  digitalWrite(AD5263_CS, LOW);

  // 7seg 
  shifted_data = HC595;
  for(i=0; i<8; i++)
  {
    // data set
    if(shifted_data & 0x80) digitalWrite(AD5263_SDI, HIGH);
    else                    digitalWrite(AD5263_SDI, LOW);
    shifted_data = (shifted_data << 1);
    // clk down -> up
    digitalWrite(AD5263_CLK, LOW);
    digitalWrite(AD5263_CLK, HIGH);
  }
  // pause for a while to identfy byte/2bit data
  for(i=0; i<8; i++) digitalWrite(AD5263_CLK, HIGH);

   // HC595
  shifted_data = SEG7;
  for(i=0; i<8; i++)
  {
    // data set
    if(shifted_data & 0x80) digitalWrite(AD5263_SDI, HIGH);
    else                    digitalWrite(AD5263_SDI, LOW);
    shifted_data = (shifted_data << 1);
    // clk down -> up
    digitalWrite(AD5263_CLK, LOW);
    digitalWrite(AD5263_CLK, HIGH);
  }
  // pause for a while to identfy byte/2bit data
  for(i=0; i<8; i++) digitalWrite(AD5263_CLK, HIGH);

  // Send AD5263_ADDR
  shifted_data = AD5263_ADDR;
  for(i=0; i<2; i++) // 2 bit address data
  {
    // data set
    if(shifted_data & 0x02) digitalWrite(AD5263_SDI, HIGH);
    else                    digitalWrite(AD5263_SDI, LOW);
    shifted_data = (shifted_data << 1);
    // clk down -> up
    digitalWrite(AD5263_CLK, LOW);
    digitalWrite(AD5263_CLK, HIGH);
  }
  // pause for a while until recognized 2bit ch data
  for(i=0; i<8; i++) digitalWrite(AD5263_CLK, HIGH);

  // Send AD5263_DATA
  shifted_data = AD5263_DATA;
  for(i=0; i<8; i++)
  {
    // data set
    if(shifted_data & 0x80) digitalWrite(AD5263_SDI, HIGH);
    else                    digitalWrite(AD5263_SDI, LOW);
    shifted_data = (shifted_data << 1);
    // clk down -> up
    digitalWrite(AD5263_CLK, LOW);
    digitalWrite(AD5263_CLK, HIGH);
  }
  // pause for a while until recognized 8bit registance data
  for(i=0; i<8; i++) digitalWrite(AD5263_CLK, HIGH);

  // latch ( change CS to HIGH )
  digitalWrite(AD5263_CS, HIGH);
 
  // termination
  digitalWrite(AD5263_SDI, HIGH);
}

void LoadPresetToCurrentParam(int prst_no)
{
  //int prev_time = appParam.time;

  appParam.volume = presetParams[prst_no].volume;
  appParam.gain = presetParams[prst_no].gain;
  appParam.treble = presetParams[prst_no].treble;
  appParam.bass = presetParams[prst_no].bass;
}
