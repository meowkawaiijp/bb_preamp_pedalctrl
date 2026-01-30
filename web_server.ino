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
  switch(key){
    case 'v':
      return UpdateClampedInt(&presetParams[index].volume, value, 0, 99);
    case 'g':
      return UpdateClampedInt(&presetParams[index].gain, value, 0, 99);
    case 't':
      return UpdateClampedInt(&presetParams[index].treble, value, -50, 50);
    case 'b':
      return UpdateClampedInt(&presetParams[index].bass, value, -50, 50);
    default:
      return false;
  }
}

// create html message sent to web browser
void CreateIndexHtml(void)
{
  char tmpstr[128];
  ResetHtmlBody();
  AppendHtmlBody(index_html1);
  for(int i=0;i<NUM_PRESET;i++)
  {
    snprintf(tmpstr, sizeof(tmpstr), "u('v%d',%d);\n", i, presetParams[i].volume);
    AppendHtmlBody(tmpstr);
    snprintf(tmpstr, sizeof(tmpstr), "u('g%d',%d);\n", i, presetParams[i].gain);
    AppendHtmlBody(tmpstr);
    snprintf(tmpstr, sizeof(tmpstr), "u('t%d',%d);\n", i, presetParams[i].treble);
    AppendHtmlBody(tmpstr);
    snprintf(tmpstr, sizeof(tmpstr), "u('b%d',%d);\n", i, presetParams[i].bass);
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
