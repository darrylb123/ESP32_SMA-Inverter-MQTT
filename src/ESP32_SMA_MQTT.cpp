/* MIT License

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
// Wifi Functions choose between Station or SoftAP

#include "ESP32_SMA_MQTT.h"

#define FORMAT_LITTLEFS_IF_FAILED


//ESP32_SMA_Inverter_App_Config& appConfigInstance = ESP32_SMA_Inverter_App_Config::getInstance();
ESP32_SMA_MQTT& mqttInstance = ESP32_SMA_MQTT::getInstance();


//link to singleton methods
extern void E_formPage() {
  ESP32_SMA_MQTT::getInstance().formPage();
}

extern void E_connectAP() {
  ESP32_SMA_MQTT::getInstance().connectAP();
}

extern void E_handleForm() {
  ESP32_SMA_MQTT::getInstance().handleForm();
}

void ESP32_SMA_MQTT::wifiTime() {
  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;

  long  gmtOffset_sec = config.timezone * 3600;   // offset seconds, this depends on your time zone (3600 is GMT +1)
  int   daylightOffset_sec = 0;  // daylight saving offset seconds
  logD("Setting time via %s, gmt: %d, dst: %d ", config.ntphostname.c_str(), gmtOffset_sec, daylightOffset_sec);


  configTime(gmtOffset_sec, daylightOffset_sec, config.ntphostname.c_str());

  String t = getTime();
  logI("Time %s" , t.c_str());

}


String ESP32_SMA_MQTT::getTime() {
  struct tm timeinfo;
  for (int i=0;i<5;i++) {
    logD(".time.");
    if(getLocalTime(&timeinfo)) {
      char charTime[64];
      strftime(charTime, sizeof(charTime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
      logD("now : %s ", charTime);
      return String(charTime);
    }
    delay(500);
  }
  logE("failed to obtain time");
  return String("");
}

void ESP32_SMA_MQTT::wifiStartup(){
  // Build Hostname
  logD("wifiStartup()");
  char sapString[20]="";
  snprintf(sapString, 20, "SMA-%08X", ESP.getEfuseMac());
  mqttInstance.sapString = String(sapString);
  logD(mqttInstance.sapString.c_str());

  // Attempt to connect to the AP stored on board, if not, start in SoftAP mode
  WiFi.mode(WIFI_STA);
  delay(2000);

  logD("setHostname");
  WiFi.hostname(mqttInstance.sapString);

#ifdef WIFI_SSID
  //overriding ssid and hostname
  logD("wifi begin with ssid(%s) and password (.......)", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for  (int w=0; w<=10 || WiFi.status() != WL_CONNECTED; w++) {
    delay(500);
    logD(".wifi.");
  }
#else
  WiFi.begin();
#endif

  //delay(2000);
  logD("Using config");

  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;
  logD("mqtt topic: %s", config.mqttTopic);
  if (config.mqttTopic == "")
    config.mqttTopic = mqttInstance.sapString;
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // Launch smartconfig to reconfigure wifi
    logD(">");
    if (i > 3)
      mySmartConfig();
    i++;
    delay(1000);
  }
  // Success connecting
  ESP32_SMA_Inverter_App::smartConfig = 0;
  String hostName = mqttInstance.sapString;
  logW("hostname %s", hostName);
  logW("IP Address: %s", ((String)WiFi.localIP().toString()).c_str());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);


  wifiTime();


  ESP32_SMA_Inverter_App::webServer.begin();
  ESP32_SMA_Inverter_App::webServer.on("/", E_formPage);
  ESP32_SMA_Inverter_App::webServer.on("/smartconfig", E_connectAP);
  ESP32_SMA_Inverter_App::webServer.on("/postform/", E_handleForm);

  logI("Web Server Running: ");

}

// Configure wifi using ESP Smartconfig app on phone
void ESP32_SMA_MQTT::mySmartConfig() {
  logD("smartConfig");
  // Wipe current credentials
  // WiFi.disconnect(true); // deletes the wifi credentials

  WiFi.mode(WIFI_STA);
  delay(2000);
  WiFi.begin();
  WiFi.beginSmartConfig();

  //Wait for SmartConfig packet from mobile
  logI("Waiting for SmartESP32_SMA_Inverter_App_Config::config");
  // if no smartconfig received after 5 minutes, reboot and try again
  int count = 0;
  while (!WiFi.smartConfigDone()) {
    delay(2000);
    logD(".");
    if (count++ > 250 ) ESP.restart();
  }
  logD("");
  logI("SmartConfig received.");

  //Wait for WiFi to connect to AP
  logW("Waiting for WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logD(".");
  }
  logW("IP Address: %s", ((String)WiFi.localIP().toString()).c_str());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  logW("Restarting in 5 seconds");
  delay(5000);
  ESP.restart();
}

// Use ESP SmartConfig to connect to wifi
void ESP32_SMA_MQTT::connectAP(){
  ESP32_SMA_Inverter_App::webServer.send(200, "text/plain", "Open ESPTouch: Smartconfig App to connect to Wifi Network");
  delay(2000);
  mySmartConfig();

}


void ESP32_SMA_MQTT::wifiLoop(){
  // Attempt to reconnect to Wifi if disconnected
  if ( WiFi.status() != WL_CONNECTED) {

    WiFi.disconnect();
    WiFi.reconnect();
  }
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - mqttInstance.previousMillis >= mqttInstance.interval)) {
    logW("Reconnecting to WiFi...\n");
    WiFi.disconnect();
    WiFi.reconnect();
    mqttInstance.previousMillis = currentMillis;
  }

  ESP32_SMA_Inverter_App::webServer.handleClient();
}

void ESP32_SMA_MQTT::formPage () {
  char tempstr[2048];
  char *responseHTML;
  InverterData& invData = ESP32_SMA_Inverter::getInstance().invData;
  DisplayData& dispData = ESP32_SMA_Inverter::getInstance().dispData;
  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;
  char fulltopic[100];

  responseHTML = (char *)malloc(sizeof(char)*10000);
  logW("Connect formpage\n");
  strcpy(responseHTML, "<!DOCTYPE html><html><head>\
                      <title>SMA Inverter</title></head><body>\
                      <style>\
                      table {\
  border-collapse: collapse;\
  width: 100%;\
  font-size: 30px;\
}\
\
table, th, td {\
  border: 1px solid black;\
}\
</style>\
<H1> SMA Bluetooth Configuration</H1>\
<form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">");


  strcat(responseHTML, "<TABLE><TR><TH>Configuration</TH><TH>Setting</TH></TR>\n");
  sprintf(tempstr, "<TR><TD>Inverter Bluetooth Address (Format AA:BB:CC:DD:EE:FF) : </TD><TD> <input type=\"text\" name=\"btaddress\" value=\"%s\"></TD><TR>\n\n",config.smaBTAddress.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Inverter Password :</TD><TD> <input type=\"text\" name=\"smapw\" value=\"%s\"></TD><TR>\n\n",config.smaInvPass.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker Hostname or IP Address :</TD><TD> <input type=\"text\" name=\"mqttBroker\" value=\"%s\"></TD><TR>\n\n",config.mqttBroker.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker Port : </TD><TD><input type=\"text\" name=\"mqttPort\" value=\"%d\"></TD><TR>\n\n",config.mqttPort);
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker User :</TD><TD> <input type=\"text\" name=\"mqttUser\" value=\"%s\"></TD><TR>\n\n",config.mqttUser.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker Password :</TD><TD> <input type=\"text\" name=\"mqttPasswd\" value=\"%s\"></TD><TR>\n\n",config.mqttPasswd.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Topic Preamble:</TD><TD> <input type=\"text\" name=\"mqttTopic\" value=\"%s\"></TD><TR>\n\n",config.mqttTopic.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>Inverter scan rate:</TD><TD> <input type=\"text\" name=\"scanRate\" value=\"%d\"></TD><TR>\n\n",config.scanRate);
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>Timezone (in hours from UTC):</TD><TD> <input type=\"text\" name=\"timezone\" value=\"%4.2f\"></TD><TR>\n\n",config.timezone);
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>NTP host:</TD><TD> <input type=\"text\" name=\"ntphostname\" value=\"%s\"></TD><TR>\n\n",config.ntphostname.c_str());
  strcat(responseHTML, tempstr);

  if (config.hassDisc) {
    strcat(responseHTML, "<TR><TD>Home Assistant Auto Discovery:</TD><TD> <input type=\"checkbox\" name=\"hassDisc\" checked ></TD><TR>\n");
    snprintf(fulltopic,sizeof(fulltopic),"homeassistant/sensor/%s/%d/state",config.mqttTopic.c_str(),invData.Serial);
  } else {
    strcat(responseHTML, "<TR><TD>Home Assistant Auto Discovery:</TD><TD> <input type=\"checkbox\" name=\"hassDisc\"></TD><TR>\n");
    snprintf(fulltopic,sizeof(fulltopic),"%s/%d/state",config.mqttTopic.c_str(),invData.Serial);
  }
  strcat(responseHTML, "</TABLE>");
  strcat(responseHTML, "<input type=\"submit\" value=\"Submit\"></form><BR> <A href=\"/smartconfig\">Enable ESP Touch App smart config</A><BR>");


  strcat(responseHTML, "<TABLE><TR><TH>Last Scan</TH><TH>Data</TH>\n");


  snprintf(tempstr, sizeof(tempstr),
"<tr><td>MQTT Topic</td><td>%s</td></tr>\n\
 <tr><td>BT Signal Strength</td><td>%4.1f %</td></tr>\n\
  <tr><td>Uac</td><td>A: %15.1f ,B: %15.1f ,C: %15.1f V</td></tr>\n\
 <tr><td>Iac</td><td>A: %15.1f ,B: %15.1f ,C: %15.1f A</td></tr>\n\
 <tr><td>Pac</td><td>%15.0f W</td></tr>\n\
 <tr><td>Udc</td><td>String 1: %15.1f V, String 2: %15.1f V</td></tr>\n\
 <tr><td>Idc</td><td>String 1: %15.1f A, String 2: %15.1f A</td></tr>\n\
 <tr><td>Wdc</td><td>String 1: %15.1f kW, String 2: %15.1f kW</td></tr>\n"
 , fulltopic
 , dispData.BTSigStrength
 , dispData.Uac[0], dispData.Uac[1], dispData.Uac[2]
 , dispData.Iac[0], dispData.Iac[1], dispData.Iac[2]
 , dispData.Pac
 , dispData.Udc[0], dispData.Udc[1]
 , dispData.Idc[0], dispData.Idc[1]
 , dispData.Udc[0] * dispData.Idc[0] / 1000 , dispData.Udc[1] * dispData.Idc[1] / 1000);


  strcat(responseHTML, tempstr);

  snprintf(tempstr, sizeof(tempstr),
"<tr><td>Frequency</td><td>%5.2f Hz</td></tr>\n\
 <tr><td>E-Today</td><td>%15.1f kWh</td></tr>\n\
 <tr><td>E-Total</td><td>%15.1f kWh</td></tr>\n "
 , dispData.Freq
 , dispData.EToday
 , dispData.ETotal);
  strcat(responseHTML, tempstr);
  strcat(responseHTML,"</TABLE></body></html>\n");
  delay(100);// Serial.print(responseHTML);
  ESP32_SMA_Inverter_App::webServer.send(200, "text/html", responseHTML);

  free(responseHTML);
}

// Function to extract the configuration
void ESP32_SMA_MQTT::handleForm() {
  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;

  logW("Connect handleForm\n");
  if (ESP32_SMA_Inverter_App::webServer.method() != HTTP_POST) {
    ESP32_SMA_Inverter_App::webServer.send(405, "text/plain", "Method Not Allowed");
  } else {
    logW("POST form was:");
    config.hassDisc = false;
    for (uint8_t i = 0; i < ESP32_SMA_Inverter_App::webServer.args(); i++) {
      String name = ESP32_SMA_Inverter_App::webServer.argName(i);
      String v = ESP32_SMA_Inverter_App::webServer.arg(i);
      v.trim();
      logW("%s: %s ",name.c_str(), v.c_str());
      if (name == "mqttBroker") {
        config.mqttBroker = v.c_str();
      } else if (name == "mqttPort") {
        String val = v.c_str();
        config.mqttPort = val.toInt();
      } else if (name == "mqttUser") {
        config.mqttUser = v.c_str();
      } else if (name == "mqttPasswd") {
        config.mqttPasswd = v.c_str();
      } else if (name == "mqttTopic") {
        config.mqttTopic = v.c_str();
      } else if (name == "btaddress") {
        config.smaBTAddress = v.c_str();
      } else if (name == "smapw") {
        config.smaInvPass = v.c_str();
      } else if (name == "scanRate") {
        config.scanRate = atoi(v.c_str());
      } else if (name == "hassDisc") {
        logW("%s\n",v.c_str());
        config.hassDisc = true;
      } else if (name == "timezone") {
        config.timezone = atof(v.c_str());
      } else if (name == "ntphostname") {
        config.ntphostname = v.c_str();
      }

    }
    ESP32_SMA_Inverter_App::getInstance().saveConfiguration();
    ESP32_SMA_Inverter_App::getInstance().printFile();
    delay(3000);
    ESP.restart();
  }
}

void ESP32_SMA_MQTT::brokerConnect() {
  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;
  if(config.mqttBroker.length() < 1 ){
    return;
  }
  logW("Connecting to MQTT Broker");

  ESP32_SMA_Inverter_App::client.setServer(config.mqttBroker.c_str(), config.mqttPort);

  // client.setCallback(callback);
  for(int i =0; i < 3;i++) {
    if ( !ESP32_SMA_Inverter_App::client.connected()){
      logW("The client %s connects to the mqtt broker %s ", mqttInstance.sapString.c_str(), config.mqttBroker.c_str());
      // If there is a user account
      if(config.mqttUser.length() > 1){
        logW(" with user/password\n");
        if (ESP32_SMA_Inverter_App::client.connect(mqttInstance.sapString.c_str(),config.mqttUser.c_str(),config.mqttPasswd.c_str())) {
        } else {
          log_e("mqtt connect failed with state %i",ESP32_SMA_Inverter_App::client.state());
          delay(2000);
        }
      } else {
        logW(" without user/password ");
        if (ESP32_SMA_Inverter_App::client.connect(mqttInstance.sapString.c_str())) {
        } else {
          log_e("mqtt connect failed with state %i", ESP32_SMA_Inverter_App::client.state());
          delay(2000);
        }
      }
    }
  }
}

// Returns true if nighttime
void ESP32_SMA_MQTT::publishData(){
  InverterData& invData = ESP32_SMA_Inverter::getInstance().invData;
  DisplayData& dispData = ESP32_SMA_Inverter::getInstance().dispData;
  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;

  if(config.mqttBroker.length() < 1 ){
    return;
  }

  brokerConnect();
  if (ESP32_SMA_Inverter_App::client.connected()){
    char theData[2000];
    // char tmpstr[100];

    // Etotal should never read less that the last good value, sometimes the value is set to 0 which causes havoc in the Home Assistant Energy display
    // Send "unavailable" instead
    if (dispData.ETotal > 10) {
      snprintf(theData,sizeof(theData)-1,
    "{ \"Serial\": %d, \"BTStrength\": %6.2f, \"Uac\": [ %6.2f, %6.2f, %6.2f ], \"Iac\": [ %6.2f, %6.2f, %6.2f ], \"Pac\": %6.0f, \"Udc\": [ %6.2f , %6.2f ], \"Idc\": [ %6.2f , %6.2f ], \"Wdc\": [%6.0f , %6.0f ], \"Freq\": %5.2f, \"EToday\": %6.2f, \"ETotal\": %15.2f, \"InvTemp\": %4.2f, \"DevStatus\": \"%s\", \"GridRelay\": \"%s\" }"
    , invData.Serial
    , dispData.BTSigStrength
    , dispData.Uac[0],dispData.Uac[1],dispData.Uac[2]
    , dispData.Iac[0],dispData.Iac[1],dispData.Iac[1]
    , dispData.Pac
    , dispData.Udc[0], dispData.Udc[1]
    , dispData.Idc[0], dispData.Idc[1]
    , dispData.Udc[0] * dispData.Idc[0] , dispData.Udc[1] * dispData.Idc[1]
    , dispData.Freq
    , dispData.EToday
    , dispData.ETotal
    , dispData.InvTemp
    , getInverterCode(invData.DevStatus).c_str()
    , getInverterCode(invData.GridRelay).c_str()
    );
    } else {
      
      snprintf(theData,sizeof(theData)-1,
    "{ \"Serial\": %d, \"BTStrength\": %6.2f, \"Uac\": [ %6.2f, %6.2f, %6.2f ], \"Iac\": [ %6.2f, %6.2f, %6.2f ], \"Pac\": %6.0f, \"Udc\": [ %6.2f , %6.2f ], \"Idc\": [ %6.2f , %6.2f ], \"Wdc\": [%6.0f , %6.0f ], \"Freq\": %5.2f, \"EToday\": %6.2f, \"ETotal\": \"unavailable\", \"InvTemp\": %4.2f, \"DevStatus\": \"%s\", \"GridRelay\": \"%s\" }"
    , invData.Serial
    , dispData.BTSigStrength
    , dispData.Uac[0],dispData.Uac[1],dispData.Uac[2]
    , dispData.Iac[0],dispData.Iac[1],dispData.Iac[1]
    , dispData.Pac
    , dispData.Udc[0], dispData.Udc[1]
    , dispData.Idc[0], dispData.Idc[1]
    , dispData.Udc[0] * dispData.Idc[0] , dispData.Udc[1] * dispData.Idc[1]
    , dispData.Freq
    , dispData.EToday
    , dispData.InvTemp
    , getInverterCode(invData.DevStatus).c_str()
    , getInverterCode(invData.GridRelay).c_str()
    );

    }


    // strcat(theData,"}");
    char topic[100];
    if (config.hassDisc)
      snprintf(topic,sizeof(topic), "sma/solar/%s-%d/state",config.mqttTopic.c_str(), config.thisSerial);
    else
      snprintf(topic,sizeof(topic), "%s-%d/state",config.mqttTopic.c_str(), config.thisSerial);
    logI("%s = %s",topic, theData);
    int len = strlen(theData);
    ESP32_SMA_Inverter_App::client.beginPublish(topic,len,false);
    if (ESP32_SMA_Inverter_App::client.print(theData))
      logI("Published\n");
    else
      logW("Failed Publish\n");
    ESP32_SMA_Inverter_App::client.endPublish();
  }
}

void ESP32_SMA_MQTT::logViaMQTT(const char *logStr){
  InverterData& invData = ESP32_SMA_Inverter::getInstance().invData;
  DisplayData& dispData = ESP32_SMA_Inverter::getInstance().dispData;
  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;

  char tmp[1000];
  if(config.mqttBroker.length() < 1 ){
    return;
  }
  snprintf(tmp,sizeof(tmp),"{ \"Log\": \"%s\" }",logStr);
  brokerConnect();

  if (ESP32_SMA_Inverter_App::client.connected()){

    // strcat(theData,"}");
    char topic[100];
    snprintf(topic,sizeof(topic), "sma/solar/%s-%d/state",config.mqttTopic.c_str(), invData.Serial);
    logI(topic);
    logI(" = ");
    logI(" %s\n",tmp);
    int len = strlen(tmp);
    ESP32_SMA_Inverter_App::client.beginPublish(topic,len,false);
    if (ESP32_SMA_Inverter_App::client.print(tmp))
      logI("Published\n");
    else
      logW("Failed Publish\n");
    ESP32_SMA_Inverter_App::client.endPublish();
  }

}


// Set up the topics in home assistant
void ESP32_SMA_MQTT::hassAutoDiscover(int timeout){

  InverterData& invData = ESP32_SMA_Inverter::getInstance().invData;
  DisplayData& dispData = ESP32_SMA_Inverter::getInstance().dispData;
  AppConfig& config = ESP32_SMA_Inverter_App::getInstance().appConfig;

  char msg[500];
  char topic[50];
  brokerConnect();

  snprintf(topic,sizeof(topic)-1, "%s-%d",config.mqttTopic.c_str(), invData.Serial);
  const size_t msg_size = sizeof(msg);

    sendHassAuto(msg, msg_size, timeout, topic, "power", "measurement", "true", "AC Power", "W", "Pac", "Pac");
    sendHassAuto(msg, msg_size, timeout, topic, "current", "measurement", "true", "A Phase Current", "A", "Iac[0]", "Iac0");
    sendHassAuto(msg, msg_size, timeout, topic, "current", "measurement", "true", "B Phase Current", "A", "Iac[1]", "Iac1");
    sendHassAuto(msg, msg_size, timeout, topic, "current", "measurement", "true", "C Phase Current", "A", "Iac[2]", "Iac2");
    sendHassAuto(msg, msg_size, timeout, topic, "voltage", "measurement", "true", "A Phase Voltage", "V", "Uac[0]", "Uac0");
    sendHassAuto(msg, msg_size, timeout, topic, "voltage", "measurement", "true", "B Phase Voltage", "V", "Uac[1]", "Uac1");
    sendHassAuto(msg, msg_size, timeout, topic, "voltage", "measurement", "true", "C Phase Voltage", "V", "Uac[2]", "Uac2");
    sendHassAuto(msg, msg_size, timeout, topic, "frequency", "measurement", "true", "AC Frequency", "Hz", "Freq", "Freq");
    sendHassAuto(msg, msg_size, timeout, topic, "power", "measurement", "true", "DC Power (String 1)", "W", "Wdc[0]", "Wdc0");
    sendHassAuto(msg, msg_size, timeout, topic, "power", "measurement", "true", "DC Power (String 2)", "W", "Wdc[1]", "Wdc1");
    sendHassAuto(msg, msg_size, timeout, topic, "voltage", "measurement", "true", "DC Voltage (String 1)", "V", "Udc[0]", "Udc0");
    sendHassAuto(msg, msg_size, timeout, topic, "voltage", "measurement", "true", "DC Voltage (String 2)", "V", "Udc[1]", "Udc1");
    sendHassAuto(msg, msg_size, timeout, topic, "current", "measurement", "true", "DC Current (String 1)", "A", "Idc[0]", "Idc0");
    sendHassAuto(msg, msg_size, timeout, topic, "current", "measurement", "true", "DC Current (String 2)", "A", "Idc[1]", "Idc1");

    sendHassAuto(msg, msg_size, timeout, topic, "energy", "total_increasing", "true", "kWh Today", "kWh", "EToday", "EToday");
    sendHassAuto(msg, msg_size, timeout, topic, "energy", "total_increasing", "true", "kWh Total", "kWh", "ETotal", "ETotal");

    sendHassAuto(msg, msg_size, timeout, topic, "temperature", "measurement", "true", "Inverter Temperature", "°C", "InvTemp", "InvTemp");
    sendHassAutoNoClassNoUnit(msg, msg_size, timeout, topic, "Device Status", "DevStatus", "DevStatus");
    sendHassAutoNoClassNoUnit(msg, msg_size, timeout, topic, "Grid Relay Status", "GridRelay", "GridRelay");
    sendHassAutoNoClass(msg, msg_size, timeout, topic, "Bluetooth", "%", "BTStrength", "BTStrength");
    sendHassAutoNoClassNoUnit(msg, msg_size, timeout, topic, "Log", "Log", "Log");

}

void ESP32_SMA_MQTT::sendHassAutoNoClassNoUnit(char *msg, size_t msg_size, int timeout, const char *topic, const char *devname, const char *sensortype, const char *sensortypeid) {
    snprintf(msg, msg_size, "{\"name\": \"%s\" , \"state_topic\": \"sma/solar/%s/state\", \"expire_after\": %d, \"value_template\": \"{{ value_json.%s }}\", \"unique_id\": \"%s-%s\" , \"device\": { \"identifiers\": [\"%s\"], \"name\": \"%s\", \"manufacturer\": \"SMA\"  } }", devname, topic, timeout, sensortype, topic, sensortypeid, topic, topic);
    logD(msg);
    sendLongMQTT(topic, sensortypeid, msg);
}

void
ESP32_SMA_MQTT::sendHassAutoNoClass(char *msg, size_t msg_size, int timeout, const char *topic, const char *devname, const char *unitOf, const char *sensortype, const char *sensortypeid) {
    snprintf(msg, msg_size, "{\"name\": \"%s\" , \"state_topic\": \"sma/solar/%s/state\", \"unit_of_measurement\": \"%s\", \"expire_after\": %d, \"value_template\": \"{{ value_json.%s }}\", \"unique_id\": \"%s-%s\" , \"device\": { \"identifiers\": [\"%s\"], \"name\": \"%s\", \"manufacturer\": \"SMA\"  } }", devname, topic, unitOf, timeout, sensortype, topic, sensortypeid, topic, topic);
    logD(msg);
    sendLongMQTT(topic, sensortypeid, msg);
}

void ESP32_SMA_MQTT::sendHassAuto(char *msg, size_t msg_size, int timeout, const char *topic, const char *devclass, const char *stateclass, const char *forceupdate,
                                  const char *devname, const char *unitOf, const char *sensortype,
                                  const char *sensortypeid) {
    snprintf(msg, msg_size, "{\"device_class\": \"%s\", \"state_class\": \"%s\", \"force_update\": \"%s\", \"name\": \"%s\" , \"state_topic\": \"sma/solar/%s/state\", \"unit_of_measurement\": \"%s\", \"expire_after\": %d, \"value_template\": \"{{ value_json.%s }}\", \"unique_id\": \"%s-%s\", \"device\": { \"identifiers\": [\"%s\"], \"name\": \"%s\", \"manufacturer\": \"SMA\"  } } ", 
      devclass, stateclass, forceupdate, devname, topic, unitOf, timeout, sensortype, topic, sensortypeid, topic, topic);
    logD(msg);
    sendLongMQTT(topic, sensortypeid, msg);
}

void ESP32_SMA_MQTT::sendLongMQTT(const char *topic, const char *postscript, const char *msg){
  int len = strlen(msg);
  char tmpstr[100];
  snprintf(tmpstr,sizeof(tmpstr),"homeassistant/sensor/%s/%s/config",topic,postscript);
  ESP32_SMA_Inverter_App::client.beginPublish(tmpstr,len,true);
  logI("%s:  %s... ",tmpstr,msg);
   if (ESP32_SMA_Inverter_App::client.print(msg))
      logI("Published\n");
    else
      logW("Failed Publish\n");
    ESP32_SMA_Inverter_App::client.endPublish();
    delay(200);
}