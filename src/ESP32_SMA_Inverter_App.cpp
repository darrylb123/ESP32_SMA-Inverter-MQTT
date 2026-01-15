/* MIT License

Copyright (c) 2022 Lupo135
Copyright (c) 2023 darrylb123

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "ESP32_SMA_Inverter_App.h"



ESP32_SMA_Inverter_App& smaInverterApp = ESP32_SMA_Inverter_App::getInstance();
ESP32_SMA_Inverter& smaInverter = ESP32_SMA_Inverter::getInstance();
ESP32_SMA_MQTT& mqttInstanceForApp = ESP32_SMA_MQTT::getInstance();

WiFiClient ESP32_SMA_Inverter_App::espClient = WiFiClient();
PubSubClient ESP32_SMA_Inverter_App::client = PubSubClient(espClient);
WebServer ESP32_SMA_Inverter_App::webServer(80);

int ESP32_SMA_Inverter_App::smartConfig = 0;

void setup() { 

  Logging::setLevel(esp32m::Info);
  Logging::addAppender(&ETSAppender::instance());
#ifdef SYSLOG_HOST
  udpappender.setMode(UDPAppender::Format::Syslog);
  Logging::addAppender(&udpappender);
#endif
  Serial.println("added appenders");
  smaInverterApp.logBuild();
  smaInverterApp.appSetup();
}

void ESP32_SMA_Inverter_App::logBuild() {
  logW("v1 Build 2w d (%s) t (%s) "  ,__DATE__ , __TIME__) ; 
}

void ESP32_SMA_Inverter_App::appSetup() { 
  Serial.begin(115200); 
  delay(1000);
  configSetup();
  mqttInstanceForApp.wifiStartup();
  
  InverterData& invData = ESP32_SMA_Inverter::getInstance().invData;
  DisplayData& dispData = ESP32_SMA_Inverter::getInstance().dispData;  


  if ( !smartConfig) {
    // Convert the MAC address string to binary
    sscanf(appConfig.smaBTAddress.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
            &smaBTAddress[0], &smaBTAddress[1], &smaBTAddress[2], &smaBTAddress[3], &smaBTAddress[4], &smaBTAddress[5]);
    // Zero the array, all unused butes must be 0
    for(int i = 0; i < sizeof(smaInvPass);i++)
       smaInvPass[i] ='\0';
    strlcpy(smaInvPass , appConfig.smaInvPass.c_str(), sizeof(smaInvPass));

    invData.SUSyID = 0x7d;
    invData.Serial = 0;
    nextTime = millis();
    // reverse inverter BT address
    for(uint8_t i=0; i<6; i++) invData.BTAddress[i] = smaBTAddress[5-i];
    logD("invData.BTAddress: %02X:%02X:%02X:%02X:%02X:%02X\n",
                invData.BTAddress[5], invData.BTAddress[4], invData.BTAddress[3],
                invData.BTAddress[2], invData.BTAddress[1], invData.BTAddress[0]);
    // *** Start BT
    smaInverter.begin("ESP32toSMA", true); // "true" creates this device as a BT Master.
  }
  // *** Start WIFI and WebServer

} 

  // **** Loop ************
void loop() { 
  smaInverterApp.appLoop();
}

void ESP32_SMA_Inverter_App::appLoop() { 
  int adjustedScanRate;
  struct tm timeinfo;
  InverterData& invData = ESP32_SMA_Inverter::getInstance().invData;
  bool ntpWorking = getLocalTime(&timeinfo);

// Check if the Sun is up or the grid relay is closed
  if ((ntpWorking && (timeinfo.tm_hour >= SUNUP) && (timeinfo.tm_hour <= SUNDOWN)) || (invData.GridRelay == 51)){
    nightTime = false;
    adjustedScanRate = (appConfig.scanRate * 1000);
  } else {
    nightTime = true;
    adjustedScanRate = NIGHTSCANRATE;
  }
  // connect or reconnect after connection lost 
  if ( !smartConfig && (nextTime < millis()) && (!smaInverter.isBtConnected())) {
    nextTime = millis() + adjustedScanRate;
    if(nightTime)
      logW("Night time - 15min scans\n");
    smaInverter.setPcktID(1);//pcktID = 1;
    
    // **** Connect SMA **********
    logW("Connecting SMA inverter: \n");
    if (smaInverter.connect(smaBTAddress)) {
      //btConnected = true;
      
      // **** Initialize SMA *******
      logW("BT connected \n");
      E_RC rc = smaInverter.initialiseSMAConnection();
      logI("SMA %d \n",rc);
      smaInverter.getBT_SignalStrength();

#ifdef LOGOFF
      // not sure the purpose but SBfSpot code logs off before logging on and this has proved very reliable for me: mrtoy-me 
      logoffSMAInverter();
#endif
      // **** logon SMA ************
      logW("*** logonSMAInverter\n");
      rc = smaInverter.logonSMAInverter(smaInvPass, USERGROUP);
      logI("Logon return code %d\n",rc);
      smaInverter.ReadCurrentData();
#ifdef LOGOFF    
      //logoff before disconnecting
      logoffSMAInverter();
#endif
      
      smaInverter.disconnect(); //moved btConnected to inverter class
      
      //Send Home Assistant autodiscover
      if(appConfig.mqttBroker.length() > 0 && appConfig.hassDisc ) { 
        if(firstTime){
          mqttInstanceForApp.hassAutoDiscover(1800); // Start with night time expire period
        //mqttInstanceForApp.hassAutoDiscover(appConfig.scanRate *2);
          mqttInstanceForApp.logViaMQTT("First boot");
          firstTime=false;
          dayNight = nightTime; // Set the flag to current state on boot
        } else if( nightTime != dayNight ) {
          if (nightTime) { // Change the expire time in home Assistant
            mqttInstanceForApp.hassAutoDiscover(1800);
            mqttInstanceForApp.logViaMQTT("Night Time");
          } else {
            mqttInstanceForApp.hassAutoDiscover(appConfig.scanRate *2);
            mqttInstanceForApp.logViaMQTT("Day Time");
          }
          dayNight = nightTime;
        }
        delay(5000);
      }

//       mqttInstanceForApp.publishData();
      failCount=0;
    } else { 
      // Inverter shuts down at night so no bluetooth. Don't bother rebooting, just keep trying 
      if (!nightTime) {
        mqttInstanceForApp.logViaMQTT("Bluetooth failed to connect");
        failCount++;
        if( failCount > 5 ) {
          logW("Failed to connect 5 times: Reboot\n");
          ESP.restart();
        }
      }
      mqttInstanceForApp.publishData();
    } 
  }
  // DEBUG1_PRINT(".");
  mqttInstanceForApp.wifiLoop();

    
  delay(100);
}


// Loads the configuration from a file
void ESP32_SMA_Inverter_App::loadConfiguration() {
  // Open file for reading
  File file = LittleFS.open("/config.txt","r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    log_e("Failed to read file, using default configuration");

  // Copy values from the JsonDocument to the Config         
  std::vector<std::string> keyNames = {"mqttBroker", "mqttPort", "mqttUser","mqttPasswd", "mqttTopic","smaInvPass", "smaBTAddress", "scanRate", "hassDisc"};
  for (uint i=0;i<keyNames.size();i++) {
    std::string k = keyNames[i];
    std::string v = doc[k];
    log_w("load key: %s , value: %s", k.c_str(), v.c_str());
  }

  #ifdef SMA_WIFI_CONFIG_VALUES_H
    appConfig.mqttBroker =  doc["mqttBroker"] | MQTT_BROKER;
    appConfig.mqttPort = doc["mqttPort"] | MQTT_PORT ;
    appConfig.mqttUser = doc["mqttUser"] | MQTT_USER;
    appConfig.mqttPasswd = doc["mqttPasswd"] | MQTT_PASS;
    appConfig.mqttTopic = doc["mqttTopic"] | MQTT_topic;
    appConfig.smaInvPass = doc["smaInvPass"] | SMA_PASS;
    appConfig.smaBTAddress = doc["smaBTAddress"] | SMA_BTADDRESS;
    appConfig.scanRate = doc["scanRate"] | SCAN_RATE ;
    appConfig.hassDisc = doc["hassDisc"] | HASS_DISCOVERY ;
    appConfig.timezone = doc["timezone"] | TIMEZONE;
    appConfig.ntphostname = doc["ntphostname"] | NTPHOSTNAME;
  #else
    appConfig.mqttBroker =  doc["mqttBroker"] | "";
    appConfig.mqttPort = doc["mqttPort"] | 1883 ;
    appConfig.mqttUser = doc["mqttUser"] | "";
    appConfig.mqttPasswd = doc["mqttPasswd"] | "";
    appConfig.mqttTopic = doc["mqttTopic"] | "SMA";
    appConfig.smaInvPass = doc["smaInvPass"] | "password";
    appConfig.smaBTAddress = doc["smaBTAddress"] | "AA:BB:CC:DD:EE:FF";
    appConfig.scanRate = doc["scanRate"] | 60 ;
    appConfig.hassDisc = doc["hassDisc"] | true ;
    appConfig.timezone = doc["timezone"] | 1;
    appConfig.ntpHostname = doc["ntphostname"] | "";
  #endif

  
  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
  
}



// Saves the configuration to a file
void ESP32_SMA_Inverter_App::saveConfiguration() {
  // Delete existing file, otherwise the configuration is appended to the file
  if (LittleFS.remove("/config.txt")) {
    log_w("removed file %s", "/config.txt");
  } else {
    log_e("failed to removed file %s", "/config.txt");
  }

  // Open file for writing
  log_i("creating file %s mode w", "/config.txt");
  File file = LittleFS.open("/config.txt", "w");
  if (!file) {
    log_e("Failed to create file");
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // Set the values in the document
  doc["mqttBroker"] = appConfig.mqttBroker;
  doc["mqttPort"] = appConfig.mqttPort;
  doc["mqttPort"] = appConfig.mqttPort;
  doc["mqttUser"] = appConfig.mqttUser;
  doc["mqttPasswd"] = appConfig.mqttPasswd;
  doc["mqttTopic"] = appConfig.mqttTopic; 
  doc["smaInvPass"] = appConfig.smaInvPass;
  doc["smaBTAddress"] = appConfig.smaBTAddress;
  doc["scanRate"] = appConfig.scanRate;
  doc["hassDisc"] = appConfig.hassDisc;
  doc["timezone"] = appConfig.timezone;
  doc["ntphostname"] = appConfig.ntphostname;

  std::vector<std::string> keyNames = {"mqttBroker", "mqttPort", "mqttUser","mqttPasswd", "mqttTopic","smaInvPass", "smaBTAddress", "scanRate", "hassDisc", "timezone", "ntphostname"};
  for (uint i=0;i<keyNames.size();i++) {
    std::string k = keyNames[i];
    std::string v = doc[k];
    log_w("save key: %s , value: %s", k.c_str(), v.c_str());
  }
 
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    log_e("Failed to write to file");
  } else {
    log_w("wrote to file");
  }

  // Close the file
  file.close();
  log_d("close file");
}

// Prints the content of a file to the Serial
void ESP32_SMA_Inverter_App::printFile() {
  // Open file for reading
  File file = LittleFS.open("/config.txt","r");
  if (!file) {
    log_e("Failed to read file");
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

void ESP32_SMA_Inverter_App::configSetup() {
  
  if (!LittleFS.begin(false)) {
    log_e("LittleFS mount failed");
    if (!LittleFS.begin(true /* true: format */)) {
      Serial.println("Failed to format LittleFS");
    } else {
      Serial.println("LittleFS formatted successfully");  return;
    }
  } else{
    log_w("little fs mount sucess");
  }

  // Should load default config if run for the first time
  log_w("Loading configuration...");
  loadConfiguration( );

  // Create configuration file
  log_w("Saving configuration...");
  saveConfiguration( );

  // Dump config file
  log_w("Print config file...");
  printFile();
}

void ESP32_SMA_Inverter_App::rmfiles(){
  if (LittleFS.remove("/config.txt")) {
    log_w("%s removed", "/config.txt");
  } else {
    log_e("%s removal failed", "/config.txt");
  }
}


