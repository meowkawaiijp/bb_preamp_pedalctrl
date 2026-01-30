// callback from Arduino web-server lib

#include "web_server.h"
#include "myUtil.h"

char htmlbody[32768];
static size_t htmlbody_len = 0;

static void ResetHtmlBody(void)
{
  htmlbody[0] = '\0';
  htmlbody_len = 0;
}

static void AppendHtmlBody(const char *text)
{
  size_t text_len = strlen(text);
  size_t available = sizeof(htmlbody) - 1 - htmlbody_len;
  if(available == 0){
    return;
  }
  if(text_len > available){
    text_len = available;
  }
  memcpy(&htmlbody[htmlbody_len], text, text_len);
  htmlbody_len += text_len;
  htmlbody[htmlbody_len] = '\0';
}

static bool UpdatePresetValue(int index, char key, int value)
{
  int minValue = 0;
  int maxValue = 0;
  int *target = NULL;

  switch(key){
    case 'v':
      minValue = 0;
      maxValue = 99;
      target = &presetParams[index].volume;
      break;
    case 'g':
      minValue = 0;
      maxValue = 99;
      target = &presetParams[index].gain;
      break;
    case 't':
      minValue = -50;
      maxValue = 50;
      target = &presetParams[index].treble;
      break;
    case 'b':
      minValue = -50;
      maxValue = 50;
      target = &presetParams[index].bass;
      break;
    default:
      return false;
  }

  if(value < minValue) value = minValue;
  if(value > maxValue) value = maxValue;
  if(*target == value){
    return false;
  }
  *target = value;
  return true;
}

// create html message sent to web browser
void CreateIndexHtml(void)
{
  char tmpstr[128];
  ResetHtmlBody();
  AppendHtmlBody(index_html1);
  for(int i=0;i<NUM_PRESET;i++)
  {
    snprintf(tmpstr, sizeof(tmpstr), "document.getElementById('v%d').value = %d;\n", i, presetParams[i].volume);
    AppendHtmlBody(tmpstr);
    snprintf(tmpstr, sizeof(tmpstr), "document.getElementById('g%d').value = %d;\n", i, presetParams[i].gain);
    AppendHtmlBody(tmpstr);
    snprintf(tmpstr, sizeof(tmpstr), "document.getElementById('t%d').value = %d;\n\n", i, presetParams[i].treble);
    AppendHtmlBody(tmpstr);
    snprintf(tmpstr, sizeof(tmpstr), "document.getElementById('b%d').value = %d;\n\n", i, presetParams[i].bass);
    AppendHtmlBody(tmpstr);
  }
  AppendHtmlBody(index_html3);
  if(debug_output_to_html){
    AppendHtmlBody("<br />\n");
    AppendHtmlBody(debug_message);
    AppendHtmlBody("<br />\n");
  }
  AppendHtmlBody(index_html4);
}

// callback when root url accessed
void handleRoot(void) {
  char uriStr[256];
  // exit
  if(appVars.available_wifi==true)
  {
    httpServer.uri().toCharArray(uriStr, 255);
    if(strcmp(uriStr, "/")==0){
      // send index.html
      CreateIndexHtml();
      httpServer.send(200, "text/html", htmlbody);
    }
  }
}

// callback when parameter change uri(/preset) accessed
void handleSetVol(void) {
  int changeCount=0;
  char uriStr[256];

  // check uri
  httpServer.uri().toCharArray(uriStr, 255);
  if(strcmp(uriStr, "/preset")==0){
    if(appVars.available_wifi==true)
    {
      for (uint8_t i = 0; i < httpServer.args(); i++) {
        //  message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
        char name[32]; 
        httpServer.argName(i).toCharArray(name, 31);
        if(strlen(name)==2){
          // check if the 1st letter is e/f/t
          if(name[0]=='v' || name[0]=='g' || name[0]=='t' || name[0]=='b'){
            // check if the 2nd letter is between 0 to 9
            if(name[1]>='0' && name[1]<='9'){
              int index = name[1]-'0';
              if(index>=0 && index<NUM_PRESET)
              {
                int value = httpServer.arg(i).toInt();
                if(UpdatePresetValue(index, name[0], value)){
                  changeCount++;
                }
              }
            }
          }
        }
      }
      
      // verify preset value
      CheckAndCorrectPreset();

      // send index html
      CreateIndexHtml();
      httpServer.send(200, "text/html", htmlbody);

      // if any preset is changed
      if(changeCount){
        // update html
        appVars.changed_via_wifi = true;

        // save to eeprom
        save_eeprom();
        // LCD
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Presets replaced");
        lcd.setCursor(0,1);
        lcd.print("via WiFi.");
        delay(3000);
      }
    }
  }
  else{
      CreateIndexHtml();
      httpServer.send(200, "text/html", htmlbody);
  }
}
