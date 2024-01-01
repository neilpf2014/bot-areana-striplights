/***   LED controller code
 *  This will set an arbetrary string of addressable LED based on JSON recived as
 *  MQTT messages
 *  Format is led # followed HSV or RGB value
 *  Alternative is array of tuples (Need to figure out how big of a message we can send)
 *  NF 20231223
*/

#include <stdlib.h>
#include <Arduino.h>
#include <PushButton.h>
#include <MQTThandler.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ESPmDNS.h>
#include <FastLED.h>

#define DEBUG_ON 1
byte debugMode = DEBUG_ON;

#define DBG(...) debugMode == DEBUG_ON ? Serial.println(__VA_ARGS__) : NULL
//Fast led stuff
// used for testing LED Strip
#define DATA_PIN 3
#define CLOCK_PIN 13
#define BTN_PIN 33

#define NUM_LEDS 1
#define LED_PERIOD 5

// Define the array of leds
CRGB leds[NUM_LEDS];
// last timer update
uint64_t LEDPastMils;
uint64_t PastMils;

// led hue parameter
uint8_t Hue;

PushButton PB1(BTN_PIN);

#define PUBSUB_DELAY 200          // ms pubsub update rate
#define ADD_TIME_BTN_DELAY 2000   // ms delay to count add time btn as "down"

#define AP_DELAY 2000
#define HARD_CODE_BROKER "192.168.1.150"
#define CONFIG_FILE "/svr-config.json"
uint64_t PubSub_timer;

//********** Wifi and MQTT stuff below ******************************************************
//******* based on Moxie board project  *****************************************************
//** Update these with values suitable for the broker used. *********************************
//** should now save param's entered in the CP screen

IPAddress MQTTIp(192, 168, 1, 140); // IP oF the MQTT broker if not 192.168.1.183

WiFiClient espClient;
uint64_t lastMsg = 0;
unsigned long MessID;

uint64_t msgPeriod = 10000; // Message check interval in ms (10 sec for testing)

String S_Stat_msg;
String S_Match;
String sBrokerIP;
int value = 0;
uint8_t GotMail;
uint8_t statusCode;
bool SaveConf_flag = false;
bool Use_def_IP_flag = false;

uint8_t ConnectedToAP = false;
MQTThandler MTQ(espClient, MQTTIp);
// change for legacy electronics on green arena
const char *outTopic = "LedStrip1";
const char *inTopic = "LEDctl1";

// used to get JSON config
uint8_t GetConfData(void)
{
  uint8_t retVal = 1;
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    if (SPIFFS.exists(CONFIG_FILE))
    {
      File CfgFile = SPIFFS.open(CONFIG_FILE, "r");
      if (CfgFile.available())
      {
        StaticJsonDocument<512> jsn;
        DeserializationError jsErr = deserializeJson(jsn, CfgFile);
        serializeJsonPretty(jsn, Serial); // think this is just going to print to serial
        if (!jsErr)
        {
          sBrokerIP = jsn["BrokerIP"].as<String>();
          retVal = 0;
        }
        else
          DBG("JSON error");
      }
    }
    else
      DBG("File_not_found");
  }
  return retVal;
}

// used to save config as JSON
uint8_t SaveConfData(String sIP)
{
  uint8_t retVal = 1;
  // SPIFFS.format();
  if (SPIFFS.begin(true))
  {

    StaticJsonDocument<512> jsn;
    jsn["BrokerIP"] = sIP;
    File CfgFile = SPIFFS.open(CONFIG_FILE, "w");
    if (CfgFile)
    {
      if (serializeJson(jsn, CfgFile) != 0)
      {
        retVal = 0;
        DBG("wrote something");
      }
      else
        DBG("failed to write file");
    }
    CfgFile.close();
  }
  else
  {
    retVal = 1;
    DBG("failed to open file");
  }

  return retVal;
}

// Wifi captive portal setup on ESP32
void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  // if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  digitalWrite(LED_BUILTIN, HIGH);
  Use_def_IP_flag = true;
}
void saveConfigCallback()
{
  // Set save config flag
  SaveConf_flag = true;
}

// Split wifi config into 2 procs to avoid code duplication
// this does most of the heavy lifting
// Saving of Params is working now !!
void WiFiCP(WiFiManager &WFM)
{
  bool validIP;
  uint8_t loadedFile;
  uint8_t savedFile;
  uint8_t isConnected;
  bool replaceHCIP;
  String sIPaddr;
  IPAddress MQTTeIP;

  WFM.setSaveConfigCallback(saveConfigCallback);
  WFM.setAPCallback(configModeCallback);
  replaceHCIP = false;
  WiFiManagerParameter TB_brokerIP("TBbroker", "MQTT broker IP", "192.168.1.150", 30);
  WFM.setHostname("ArenaLights_1");
  WFM.addParameter(&TB_brokerIP);
  isConnected = WFM.autoConnect("LightConfigAP");
  if (isConnected)
  {
    DBG("Connected");
    loadedFile = GetConfData();
    // load from file ignore TB
    if (!Use_def_IP_flag)
    {
      DBG("loaded IP from File");
      validIP = MQTTeIP.fromString(sBrokerIP);
      if (validIP)
        replaceHCIP = true;
    }
    else
    {
      sIPaddr = TB_brokerIP.getValue();
      DBG("Used IP from TB");
      if (!sIPaddr.isEmpty())
      {
        validIP = MQTTeIP.fromString(sIPaddr);
        if (validIP)
        {
          replaceHCIP = true;
          if (SaveConf_flag == true)
          {
            sBrokerIP = sIPaddr;
            SaveConf_flag = SaveConfData(sIPaddr);
          }
        }
      }
    }
    if (replaceHCIP == true)
    {
      MQTTIp = MQTTeIP;
      DBG("replaced default");
    }
  }
}

// called to set up wifi -- Part 1 of 2
void WiFiConf(uint8_t ResetAP)
{
  WiFiManager wifiManager;
  if (ResetAP)
  {
    wifiManager.resetSettings();
    WiFiCP(wifiManager);
  }
  else
    WiFiCP(wifiManager);
  // these are used for debug
  Serial.println("Print IP:");
  Serial.println(WiFi.localIP());
  // **************************
  GotMail = false;
  MTQ.setClientName("ESP32Client");
  MTQ.subscribeIncomming(inTopic);
  MTQ.subscribeOutgoing(outTopic);
}


// **********************************************************************************************
// ****************** End Wifi Config code ******************************************************


// separate the Wifi / MQTT init from other setup stuff
void IOTsetup()
{
  bool testIP;
  uint64_t APmodeCKtimer;
  uint8_t Btnstate;
  uint8_t tempint;
  APmodeCKtimer = millis();
  Btnstate = 0;

  // Will wait 2 sec and check for reset to be held down / pressed
  while ((APmodeCKtimer + AP_DELAY) > millis())
  {
    if (PB1.isCycled())
      Btnstate = 1;
    PB1.update();
  }
  tempint = PB1.cycleCount();
  String TempIP = MQTTIp.toString();
  // these lines set up the access point, mqtt & other internet stuff
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the Green for Wifi
  WiFiConf(Btnstate);
  // comment this out so we can enter broker IP on setup
  /*
  testIP = mDNShelper();
  if (!testIP){
    MQTTIp.fromString(TempIP);
  }
  Serial.print("IP address of server: ");
  */
  Serial.print("IP address of broker: ");
  Serial.println(MQTTIp.toString());
  MTQ.setServerIP(MQTTIp);
  digitalWrite(LED_BUILTIN, LOW); // turn off the light if on from config
  // **********************************************************
}

// Added to output to MQTT as JSON
String MakeJson(uint32_t iMatchSCode, String SMatch, uint64_t SecRemain, uint32_t Mlen)
{
  StaticJsonDocument<256> JM;
  String sJSoutput;
  JM["Mills"] = millis();
  JM["Match_State_Code"] = iMatchSCode;
  JM["Match_State"] = SMatch;
  JM["Match_Sec_remain"] = SecRemain;
  JM["Match_Length"] = Mlen;
  serializeJson(JM, sJSoutput);
  return sJSoutput;
}

void setup()
{

  Serial.begin(115200);
  IOTsetup();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  Hue = 0;

  
  Serial.println("program starting");
  delay(1000);
  
  gSDtimer = 0;
  
  Btn_timer = millis();
  PubSub_timer = millis();
}

void loop() {
  // color cycle the led
  if (millis() > LEDPastMils + LED_PERIOD)
  {
      if (Hue > 254)
        Hue = 0;
      for (int i=0; i > NUM_LEDS; i++)
        leds[i] = CHSV(Hue++,255,127);
      FastLED.show();
      PastMils = millis();
  }

  // Deal with MQTT Pubsub
  if ((millis() - PubSub_timer) > PUBSUB_DELAY)
  {
    // send / recieve status via MQTT
    S_Match = MstateSetMQTT(g_match, g_Match_Reset, 0);
    // Uncomment to send string the old way
    // S_Stat_msg = String(millis()) + "," + String(g_match) + "," + S_Match + "," + String(MatchSecRemain) + "," + String(MATCH_LEN);
    // Send as JSON now
    if (g_match != MatchState::starting)
    {
      S_Stat_msg = MakeJson(int(g_match), S_Match, MatchSecRemain, MATCH_LEN);
      MQstatcode = MTQ.publish(S_Stat_msg);
    }

    GotMail = MTQ.update();
    if (GotMail == true)
    {
      //*********Incoming Msg *****************************
      Serial.print("message is: ");
      Msgcontents = MTQ.GetMsg();
      Serial.println(Msgcontents);
      MQTThandleIncoming(Msgcontents, ResetSec, g_match, g_Match_Reset);
      //********************************************************
      GotMail = false;
    }
    PubSub_timer = millis();
  }
}
