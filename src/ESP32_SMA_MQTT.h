#pragma once 
#ifndef ESP32_SMA_MQTT_H
#define ESP32_SMA_MQTT_H




#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include <Preferences.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <map>

#include "SMA_Inverter.h"
#include "SMA_Utils.h"
#include "ESP32_SMA_Inverter_App.h"
#include "config_values.h"
#include "ESP32Loggable.h"


class ESP32_SMA_MQTT : public ESP32Loggable  {

public:
    // Static method to get the instance of the class.
    static ESP32_SMA_MQTT& getInstance() {
        // This guarantees that the instance is created only once.
        static ESP32_SMA_MQTT instance;
        return instance;
    }

    // Delete the copy constructor and the assignment operator to prevent cloning.
    ESP32_SMA_MQTT(const ESP32_SMA_MQTT&) = delete;
    ESP32_SMA_MQTT& operator=(const ESP32_SMA_MQTT&) = delete;

    // Your class methods and members go here.
    //Prototypes
     void wifiStartup();
     void wifiTime();
     void mySmartConfig();
     void connectAP();
     void wifiLoop();
     void formPage ();
     void handleForm();
     void brokerConnect();
     void publishData();
     void hassAutoDiscover(int timeout);
     void sendLongMQTT(const char *topic, const char *postscript, const char *msg);
     void logViaMQTT(const char *logStr);

    String getTime();

    String sapString = "";
    unsigned long previousMillis = 0;
    unsigned long interval = 30000;

protected:
    std::map<int, std::string> codeMap;

private:
    // Private constructor to prevent instantiation from outside the class.
    ESP32_SMA_MQTT() : ESP32Loggable("ESP32_SMA_MQTT") {
         initMap();
        logger().setLevel(esp32m::Debug);
    }

    // Destructor (optional, as the singleton instance will be destroyed when the program ends).
    ~ESP32_SMA_MQTT() {}

    // Inverter index decoding
      void initMap() {

      codeMap[50]="Status";
      codeMap[51]="Closed";

      codeMap[300]="Nat";
      codeMap[301]="Grid failure";
      codeMap[302]="-------";
      codeMap[303]="Off";
      codeMap[304]="Island mode";
      codeMap[305]="Island mode";
      codeMap[306]="SMA Island mode 60 Hz";
      codeMap[307]="OK";
      codeMap[308]="On";
      codeMap[309]="Operation";
      codeMap[310]="General operating mode";
      codeMap[311]="Open";
      codeMap[312]="Phase assignment";
      codeMap[313]="SMA Island mode 50 Hz";

      codeMap[16777213]="Information not available";

    }


    std::string getInverterCode(int invCode) {
      std::map<int, std::string>::iterator it = codeMap.find(invCode);
      if (it != codeMap.end())
        return it->second;
      else
        return std::to_string(invCode);
    }



    void sendSensorValue(char *tmpstr, const char *topic, const int timeout);

    void sendHassAuto(char *msg, size_t msg_size, int timeout, const char *topic, const char *devclass, const char *stateclass, const char *forceupdate,
                                      const char *devname, const char *unitOf, const char *sensortype,
                                      const char *sensortypeid);
    void sendHassAutoNoClassNoUnit(char *msg, size_t msg_size, int timeout, const char *topic,
                                   const char *devname, const char *sensortype, const char *sensortypeid);
    void sendHassAutoNoClass(char *msg, size_t msg_size, int timeout, const char *topic, const char *devname,
                             const char *unitOf, const char *sensortype, const char *sensortypeid);

};


extern void E_formPage();
extern void E_connectAP();
extern void E_handleForm();


#endif