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

#include <Arduino.h>
#include <Esp.h>
#include <BluetoothSerial.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <PubSubClient.h>
//#define RX_QUEUE_SIZE 2048
//#define TX_QUEUE_SIZE 64
#include <logging.hpp>
#include <ets-appender.hpp>
#include <udp-appender.hpp>

#include "SMA_Utils.h"
#include "SMA_bluetooth.h"
#include "SMA_Inverter.h"

#include "MQTT.h"
#include "ESP32_SMA_Inverter_MQTT.h"

ESP32_SMA_Inverter_App smaInverterApp = ESP32_SMA_Inverter_App();
ESP32_SMA_Inverter& smaInverter = ESP32_SMA_Inverter::getInstance();
//ESP32_SMA_MQTT mqtt = ESP32_SMA_MQTT();

uint32_t ESP32_SMA_Inverter_App::appSerial=0;
WiFiClient ESP32_SMA_Inverter_App::espClient = WiFiClient();
PubSubClient ESP32_SMA_Inverter_App::client = PubSubClient(espClient);
WebServer ESP32_SMA_Inverter_App::webServer(80);

ESP32_SMA_MQTT& mqttInstanceForApp = ESP32_SMA_MQTT::getInstance();
ESP32_SMA_Inverter_App_Config& inverterAppConfig =  ESP32_SMA_Inverter_App_Config::getInstance();

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
  inverterAppConfig.configSetup();
  mqttInstanceForApp.wifiStartup();
  
  InverterData& invData = ESP32_SMA_Inverter::getInstance().invData;
  DisplayData& dispData = ESP32_SMA_Inverter::getInstance().dispData;  
  AppConfig& config = ESP32_SMA_Inverter_App_Config::getInstance().appConfig;


  if ( !smartConfig) {
    // Convert the MAC address string to binary
    sscanf(config.smaBTAddress.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
            &smaBTAddress[0], &smaBTAddress[1], &smaBTAddress[2], &smaBTAddress[3], &smaBTAddress[4], &smaBTAddress[5]);
    // Zero the array, all unused butes must be 0
    for(int i = 0; i < sizeof(smaInvPass);i++)
       smaInvPass[i] ='\0';
    strlcpy(smaInvPass , config.smaInvPass.c_str(), sizeof(smaInvPass));

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
  // connect or reconnect after connection lost 
  if (nightTime)  // Scan every 15min
    adjustedScanRate = 900000;
  else
    adjustedScanRate = (config.scanRate * 1000);
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
      if(config.mqttBroker.length() > 0 && config.hassDisc && firstTime){
        mqttInstanceForApp.hassAutoDiscover();
        mqttInstanceForApp.logViaMQTT("First boot");
        firstTime=false;
        delay(5000);
      }

      nightTime = mqttInstanceForApp.publishData();
    } else {  
      mqttInstanceForApp.logViaMQTT("Bluetooth failed to connect");
    } 
  }
  // DEBUG1_PRINT(".");
  mqttInstanceForApp.wifiLoop();
    
  delay(100);
}



