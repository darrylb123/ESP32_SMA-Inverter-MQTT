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

#include <Arduino.h>
#include "MQTT.h"
#include "SMA_Inverter.h"
#include "SMA_Utils.h"
#include "ESP32_SMA_Inverter_MQTT.h"
#include "ESP32_SMA_Inverter_App_Config.h"

#define FORMAT_LITTLEFS_IF_FAILED 


ESP32_SMA_Inverter_App_Config& appConfigInstance = ESP32_SMA_Inverter_App_Config::getInstance();
ESP32_SMA_MQTT& mqttInstance = ESP32_SMA_MQTT::getInstance();

void ESP32_SMA_MQTT::wifiStartup(){
  // Build Hostname
  snprintf(mqttInstance.sapString, 20, "SMA-%08X", ESP.getEfuseMac());

  // Attempt to connect to the AP stored on board, if not, start in SoftAP mode
  WiFi.mode(WIFI_STA);
  delay(2000);
  WiFi.hostname(mqttInstance.sapString);
  WiFi.begin();
  delay(5000);
  AppConfig* config = appConfigInstance.pConfig;
  if (config->mqttTopic == "") 
    config->mqttTopic = mqttInstance.sapString;
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // Launch smartconfig to reconfigure wifi
    Serial.print(">");
    if (i > 3)
      mySmartConfig();
    i++;
    delay(1000);
  }
  // Success connecting 
  ESP32_SMA_Inverter_App::smartConfig = 0; 
  String hostName = "Hostname: ";
  hostName = hostName + mqttInstance.sapString;
  Serial.println(hostName);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
   
  
  ESP32_SMA_Inverter_App::webServer.begin();
  ESP32_SMA_Inverter_App::webServer.on("/", formPage);
  ESP32_SMA_Inverter_App::webServer.on("/smartconfig", connectAP);
  ESP32_SMA_Inverter_App::webServer.on("/postform/",handleForm);

  Serial.print("Web Server Running: ");
  
}

// Configure wifi using ESP Smartconfig app on phone
void ESP32_SMA_MQTT::mySmartConfig() {
  // Wipe current credentials
  // WiFi.disconnect(true); // deletes the wifi credentials
  
  WiFi.mode(WIFI_STA);
  delay(2000);
  WiFi.begin();
  WiFi.beginSmartConfig();

  //Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartESP32_SMA_Inverter_App_Config::config");
  // if no smartconfig received after 5 minutes, reboot and try again
  int count = 0;
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
    if (count++ > 600 ) ESP.restart();
  }



  Serial.println("");
  Serial.println("SmartConfig received.");

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.println("Restarting in 5 seconds");
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
    DEBUG1_PRINT("Reconnecting to WiFi...\n");
    WiFi.disconnect();
    WiFi.reconnect();
    mqttInstance.previousMillis = currentMillis;
  }
  ESP32_SMA_Inverter_App::webServer.handleClient();  
}

void ESP32_SMA_MQTT::formPage () {
  char tempstr[2048];
  char *responseHTML;
  InverterData *pInvData = ESP32_SMA_Inverter::pInvData;
  DisplayData *pDispData = ESP32_SMA_Inverter::pDispData;  
  AppConfig *config = ESP32_SMA_Inverter_App_Config::getInstance().pConfig;
  char fulltopic[100];

  responseHTML = (char *)malloc(10000);
  DEBUG1_PRINT("Connect formpage\n");
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
  sprintf(tempstr, "<TR><TD>Inverter Bluetooth Address (Format AA:BB:CC:DD:EE:FF) : </TD><TD> <input type=\"text\" name=\"btaddress\" value=\"%s\"></TD><TR>\n\n",config->smaBTAddress.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Inverter Password :</TD><TD> <input type=\"text\" name=\"smapw\" value=\"%s\"></TD><TR>\n\n",config->smaInvPass.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker Hostname or IP Address :</TD><TD> <input type=\"text\" name=\"mqttBroker\" value=\"%s\"></TD><TR>\n\n",config->mqttBroker.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker Port : </TD><TD><input type=\"text\" name=\"mqttPort\" value=\"%d\"></TD><TR>\n\n",config->mqttPort);
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker User :</TD><TD> <input type=\"text\" name=\"mqttUser\" value=\"%s\"></TD><TR>\n\n",config->mqttUser.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Broker Password :</TD><TD> <input type=\"text\" name=\"mqttPasswd\" value=\"%s\"></TD><TR>\n\n",config->mqttPasswd.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>MQTT Topic Preamble:</TD><TD> <input type=\"text\" name=\"mqttTopic\" value=\"%s\"></TD><TR>\n\n",config->mqttTopic.c_str());
  strcat(responseHTML, tempstr);
  sprintf(tempstr, "<TR><TD>Inverter scan rate:</TD><TD> <input type=\"text\" name=\"scanRate\" value=\"%d\"></TD><TR>\n\n",config->scanRate);
  strcat(responseHTML, tempstr);

  if (config->hassDisc) {
    strcat(responseHTML, "<TR><TD>Home Assistant Auto Discovery:</TD><TD> <input type=\"checkbox\" name=\"hassDisc\" checked ></TD><TR>\n");
    snprintf(fulltopic,sizeof(fulltopic),"homeassistant/sensor/%s-%d/state",config->mqttTopic.c_str(),pInvData->Serial);
  } else {
    strcat(responseHTML, "<TR><TD>Home Assistant Auto Discovery:</TD><TD> <input type=\"checkbox\" name=\"hassDisc\"></TD><TR>\n");
    snprintf(fulltopic,sizeof(fulltopic),"%s-%d/state",config->mqttTopic.c_str(),pInvData->Serial);
  }
  strcat(responseHTML, "</TABLE>");
  strcat(responseHTML, "<input type=\"submit\" value=\"Submit\"></form><BR> <A href=\"/smartconfig\">Enable ESP Touch App smart config</A><BR>");


  strcat(responseHTML, "<TABLE><TR><TH>Last Scan</TH><TH>Data</TH>\n");
  
  
  snprintf(tempstr, sizeof(tempstr),
"<tr><td>MQTT Topic</td><td>%s</td></tr>\n\
 <tr><td>BT Signal Strength</td><td>%4.1f %</td></tr>\n\
  <tr><td>Uac</td><td>%15.1f V</td></tr>\n\
 <tr><td>Iac</td><td>%15.1f A</td></tr>\n\
 <tr><td>Pac</td><td>%15.1f kW</td></tr>\n\
 <tr><td>Udc</td><td>String 1: %15.1f V, String 2: %15.1f V</td></tr>\n\
 <tr><td>Idc</td><td>String 1: %15.1f A, String 2: %15.1f A</td></tr>\n\
 <tr><td>Wdc</td><td>String 1: %15.1f kW, String 2: %15.1f kW</td></tr>\n"
 , fulltopic
 , pDispData->BTSigStrength
 , pDispData->Uac
 , pDispData->Iac
 , pDispData->Pac
 , pDispData->Udc[0], pDispData->Udc[1]
 , pDispData->Idc[0], pDispData->Idc[1]
 , pDispData->Udc[0] * pDispData->Idc[0] / 1000 , pDispData->Udc[1] * pDispData->Idc[1] / 1000);

  strcat(responseHTML, tempstr);

  snprintf(tempstr, sizeof(tempstr),
"<tr><td>Frequency</td><td>%5.2f kWh</td></tr>\n\
 <tr><td>E-Today</td><td>%15.1f kWh</td></tr>\n\
 <tr><td>E-Total</td><td>%15.1f kWh</td></tr>\n "
 , pDispData->Freq
 , pDispData->EToday
 , pDispData->ETotal);
  strcat(responseHTML, tempstr);
  strcat(responseHTML,"</TABLE></body></html>\n");
  delay(100);// Serial.print(responseHTML);
  ESP32_SMA_Inverter_App::webServer.send(200, "text/html", responseHTML);

}

// Function to extract the configuration
void ESP32_SMA_MQTT::handleForm() {
  AppConfig *config = ESP32_SMA_Inverter_App_Config::getInstance().pConfig;

  DEBUG1_PRINT("Connect handleForm\n");
  if (ESP32_SMA_Inverter_App::webServer.method() != HTTP_POST) {
    ESP32_SMA_Inverter_App::webServer.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "POST form was:\n";
    config->hassDisc = false; 
    for (uint8_t i = 0; i < ESP32_SMA_Inverter_App::webServer.args(); i++) {
      String name = ESP32_SMA_Inverter_App::webServer.argName(i);
      DEBUG1_PRINTF("%s, ",name.c_str());
      if (name == "mqttBroker") {
        config->mqttBroker = ESP32_SMA_Inverter_App::webServer.arg(i);
        config->mqttBroker.trim();
      } else if (name == "mqttPort") {
        String val = ESP32_SMA_Inverter_App::webServer.arg(i);
        config->mqttPort = val.toInt();   
      } else if (name == "mqttUser") {
        config->mqttUser = ESP32_SMA_Inverter_App::webServer.arg(i);
        config->mqttUser.trim();
      } else if (name == "mqttPasswd") {
        config->mqttPasswd = ESP32_SMA_Inverter_App::webServer.arg(i);
        config->mqttPasswd.trim();
      } else if (name == "mqttTopic") {
        config->mqttTopic = ESP32_SMA_Inverter_App::webServer.arg(i);
        config->mqttTopic.trim();
      } else if (name == "btaddress") {
        config->smaBTAddress = ESP32_SMA_Inverter_App::webServer.arg(i);
        config->smaBTAddress.trim();
      } else if (name == "smapw") {
        config->smaInvPass = ESP32_SMA_Inverter_App::webServer.arg(i);
        config->smaInvPass.trim();
      } else if (name == "scanRate") {
        config->scanRate = atoi(ESP32_SMA_Inverter_App::webServer.arg(i).c_str());
      } else if (name == "hassDisc") {
        DEBUG1_PRINTF("%s\n",ESP32_SMA_Inverter_App::webServer.arg(i).c_str());
        config->hassDisc = true;
      } 

    }
    ESP32_SMA_Inverter_App_Config::getInstance().saveConfiguration(confFile);
    ESP32_SMA_Inverter_App_Config::getInstance().printFile(confFile);
    delay(5000);
    ESP.restart();
  }
}

void ESP32_SMA_MQTT::brokerConnect() {
  AppConfig *config = ESP32_SMA_Inverter_App_Config::getInstance().pConfig;
  if(config->mqttBroker.length() < 1 ){
    return;
  }
  DEBUG1_PRINT("\nConnecting to MQTT Broker\n");

  ESP32_SMA_Inverter_App::client.setServer(config->mqttBroker.c_str(), config->mqttPort);

  // client.setCallback(callback);
  for(int i =0; i < 3;i++) {
    if ( !ESP32_SMA_Inverter_App::client.connected()){
      DEBUG1_PRINTF("The client %s connects to the mqtt broker %s ", sapString,config->mqttBroker.c_str());
      // If there is a user account
      if(config->mqttUser.length() > 1){
        DEBUG1_PRINT(" with user/password\n");
        if (ESP32_SMA_Inverter_App::client.connect(mqttInstance.sapString,config->mqttUser.c_str(),config->mqttPasswd.c_str())) {
        } else {
          Serial.print("mqtt connect failed with state ");
          Serial.print(ESP32_SMA_Inverter_App::client.state());
          delay(2000);
        }
      } else {
        DEBUG1_PRINT(" without user/password\n");
        if (ESP32_SMA_Inverter_App::client.connect(mqttInstance.sapString)) {
        } else {
          Serial.print("mqtt connect failed with state ");
          Serial.print(ESP32_SMA_Inverter_App::client.state());
          delay(2000);
        }
      }
    }
  }
}

// Returns true if nighttime
bool ESP32_SMA_MQTT::publishData(){
   InverterData *pInvData = ESP32_SMA_Inverter::pInvData;
   DisplayData *pDispData = ESP32_SMA_Inverter::pDispData;
   AppConfig *config = ESP32_SMA_Inverter_App_Config::getInstance().pConfig;

  if(config->mqttBroker.length() < 1 ){
    return(false);
  }

  brokerConnect();
  if (ESP32_SMA_Inverter_App::client.connected()){
    char theData[2000];
    // char tmpstr[100];


    snprintf(theData,sizeof(theData)-1,
    "{ \"Serial\": %d, \"BTStrength\": %6.2f, \"Uac\": [ %6.2f, %6.2f, %6.2f ], \"Iac\": [ %6.2f, %6.2f, %6.2f ], \"Pac\": %6.2f, \"Udc\": [ %6.2f , %6.2f ], \"Idc\": [ %6.2f , %6.2f ], \"Wdc\": [%6.2f , %6.2f ], \"Freq\": %5.2f, \"EToday\": %6.2f, \"ETotal\": %15.2f, \"InvTemp\": %4.2f, \"DevStatus\": %d, \"GridRelay\": %d }"
 , pInvData->Serial
 , pDispData->BTSigStrength
 , pDispData->Uac[0],pDispData->Uac[1],pDispData->Uac[2]
 , pDispData->Iac[0],pDispData->Iac[1],pDispData->Iac[1]
 , pDispData->Pac
 , pDispData->Udc[0], pDispData->Udc[1]
 , pDispData->Idc[0], pDispData->Idc[1]
 , pDispData->Udc[0] * pDispData->Idc[0] / 1000 , pDispData->Udc[1] * pDispData->Idc[1] / 1000
 , pDispData->Freq
 , pDispData->EToday
 , pDispData->ETotal
 , pDispData->InvTemp
 , pInvData->DevStatus
 , pInvData->GridRelay
);


    // strcat(theData,"}");
    char topic[100];
    if (config->hassDisc)
      snprintf(topic,sizeof(topic), "homeassistant/sensor/%s-%d/state",config->mqttTopic.c_str(), pInvData->Serial);
    else
      snprintf(topic,sizeof(topic), "%s-%d/state",config->mqttTopic.c_str(), pInvData->Serial);
    DEBUG1_PRINT(topic);
    DEBUG1_PRINT(" = ");
    DEBUG1_PRINTF("%s\n",theData);
    int len = strlen(theData);
    ESP32_SMA_Inverter_App::client.beginPublish(topic,len,false);
    if (ESP32_SMA_Inverter_App::client.print(theData))
      DEBUG1_PRINT("Published\n");
    else
      DEBUG1_PRINT("Failed Publish\n");
    ESP32_SMA_Inverter_App::client.endPublish();
  }
  // If Power is zero, it's night time
  if (pDispData->Pac > 0) 
    return(false);
  else
    return(true);
}

void ESP32_SMA_MQTT::logViaMQTT(char *logStr){
  AppConfig *config = appConfigInstance.pConfig;
  InverterData *pInvData = ESP32_SMA_Inverter::pInvData; //extern InverterData *pInvData;
  char tmp[1000];
  if(config->mqttBroker.length() < 1 ){
    return;
  }
  snprintf(tmp,sizeof(tmp),"{ \"Log\": \"%s\" }",logStr);
  brokerConnect();
  
  if (ESP32_SMA_Inverter_App::client.connected()){

    // strcat(theData,"}");
    char topic[100];
    snprintf(topic,sizeof(topic), "homeassistant/sensor/%s-%d/state",config->mqttTopic.c_str(), pInvData->Serial);
    DEBUG1_PRINT(topic);
    DEBUG1_PRINT(" = ");
    DEBUG1_PRINTF(" %s\n",tmp);
    int len = strlen(tmp);
    ESP32_SMA_Inverter_App::client.beginPublish(topic,len,false);
    if (ESP32_SMA_Inverter_App::client.print(tmp))
      DEBUG1_PRINT("Published\n");
    else
      DEBUG1_PRINT("Failed Publish\n");
    ESP32_SMA_Inverter_App::client.endPublish();
  }
  
}


// Set up the topics in home assistant
void ESP32_SMA_MQTT::hassAutoDiscover(){
  AppConfig *config = appConfigInstance.pConfig;

  char tmpstr[1000];
  char topic[30];
  InverterData *pInvData = ESP32_SMA_Inverter::pInvData; //extern InverterData *pInvData;
  brokerConnect();
  
  snprintf(topic,sizeof(topic)-1, "%s-%d",config->mqttTopic.c_str(), pInvData->Serial);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"power\", \"name\": \"%s AC Power\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"kW\", \"value_template\": \"{{ value_json.Pac }}\" }",topic,topic);
  sendLongMQTT(topic,"Pac",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"current\", \"name\": \"%s A Phase Current\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"A\", \"value_template\": \"{{ value_json.Iac[0] }}\" }",topic,topic);
  sendLongMQTT(topic,"IacA",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"current\", \"name\": \"%s B Phase Current\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"A\", \"value_template\": \"{{ value_json.Iac[1] }}\" }",topic,topic);
  sendLongMQTT(topic,"IacB",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"current\", \"name\": \"%s C Phase Current\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"A\", \"value_template\": \"{{ value_json.Iac[2] }}\" }",topic,topic);
  sendLongMQTT(topic,"IacC",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"voltage\", \"name\": \"%s A Phase Voltage\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"V\", \"value_template\": \"{{ value_json.Uac[0] }}\" }",topic,topic);
  sendLongMQTT(topic,"UacA",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"voltage\", \"name\": \"%s B Phase Voltage\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"V\", \"value_template\": \"{{ value_json.Uac[1] }}\" }",topic,topic);
  sendLongMQTT(topic,"UacB",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"voltage\", \"name\": \"%s C Phase Voltage\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"V\", \"value_template\": \"{{ value_json.Uac[2] }}\" }",topic,topic);
  sendLongMQTT(topic,"UacC",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"frequency\", \"name\": \"%s AC Frequency\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"Hz\", \"value_template\": \"{{ value_json.Freq }}\" }",topic,topic);
  sendLongMQTT(topic,"Freq",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"power\", \"name\": \"%s DC Power (String 1)\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"kW\", \"value_template\": \"{{ value_json.Wdc[0] }}\" }",topic,topic);
  sendLongMQTT(topic,"Wdc1",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"power\", \"name\": \"%s DC Power (String 2)\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"kW\", \"value_template\": \"{{ value_json.Wdc[1] }}\" }",topic,topic);
  sendLongMQTT(topic,"Wdc2",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"voltage\", \"name\": \"%s DC Voltage (String 1)\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"V\", \"value_template\": \"{{ value_json.Udc[0] }}\" }",topic,topic);
  sendLongMQTT(topic,"Udc1",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"voltage\", \"name\": \"%s DC Voltage (String 2)\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"V\", \"value_template\": \"{{ value_json.Udc[1] }}\" }",topic,topic);
  sendLongMQTT(topic,"Udc2",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"current\", \"name\": \"%s DC Current (String 1)\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"A\", \"value_template\": \"{{ value_json.Idc[0] }}\" }",topic,topic);
  sendLongMQTT(topic,"Idc1",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"current\", \"name\": \"%s DC Current (String 2)\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"A\", \"value_template\": \"{{ value_json.Idc[1] }}\" }",topic,topic);
  sendLongMQTT(topic,"Idc2",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"energy\", \"name\": \"%s kWh Today\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"kWh\", \"value_template\": \"{{ value_json.EToday }}\" }",topic,topic);
  sendLongMQTT(topic,"EToday",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"energy\", \"name\": \"%s kWh Total\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"kWh\", \"value_template\": \"{{ value_json.ETotal }}\" }",topic,topic);
  sendLongMQTT(topic,"ETotal",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"device_class\": \"temperature\", \"name\": \"%s Inverter Temperature\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"unit_of_measurement\": \"C\", \"value_template\": \"{{ value_json.InvTemp }}\" }",topic,topic);
  sendLongMQTT(topic,"InvTemp",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"name\": \"%s Device Status\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"value_template\": \"{{ value_json.DevStatus }}\" }",topic,topic);
  sendLongMQTT(topic,"DevStatus",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"name\": \"%s Grid Relay Status\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"value_template\": \"{{ value_json.GridRelay }}\" }",topic,topic);
  sendLongMQTT(topic,"GridRelay",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"name\": \"%s Bluetooth\" , \"state_topic\": \"homeassistant/sensor/%s/state\",\"unit_of_measurement\": \"%%\", \"value_template\": \"{{ value_json.BTStrength }}\" }",topic,topic);
  sendLongMQTT(topic,"Bluetooth",tmpstr);
  snprintf(tmpstr,sizeof(tmpstr)-1, "{\"name\": \"%s Log\" , \"state_topic\": \"homeassistant/sensor/%s/state\", \"value_template\": \"{{ value_json.Log }}\" }",topic,topic);
  sendLongMQTT(topic,"Log",tmpstr);
}

void ESP32_SMA_MQTT::sendLongMQTT(char *topic, char *postscript, char *msg){
  int len = strlen(msg);
  char tmpstr[100];
  snprintf(tmpstr,sizeof(tmpstr),"homeassistant/sensor/%s-%s/config",topic,postscript);
  ESP32_SMA_Inverter_App::client.beginPublish(tmpstr,len,true);
  DEBUG1_PRINTF("%s -> %s... ",tmpstr,msg);
   if (ESP32_SMA_Inverter_App::client.print(msg))
      DEBUG1_PRINT("Published\n");
    else
      DEBUG1_PRINT("Failed Publish\n");
    ESP32_SMA_Inverter_App::client.endPublish();
    delay(200);
}