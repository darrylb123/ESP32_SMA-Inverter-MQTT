#pragma once 
#ifndef ESP32_SMA_MQTT_H
#define ESP32_SMA_MQTT_H


#include <WiFi.h>

#include <Preferences.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <WebServer.h>
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
     void mySmartConfig();
     void connectAP();
     void wifiLoop();
     void formPage ();
     void handleForm();
     void brokerConnect();
     bool publishData();
     void hassAutoDiscover();
     void sendLongMQTT(const char *topic, const char *postscript, const char *msg);
     void logViaMQTT(const char *logStr);

    String sapString = "";
    unsigned long previousMillis = 0;
    unsigned long interval = 30000;

private:
    // Private constructor to prevent instantiation from outside the class.
    ESP32_SMA_MQTT() : ESP32Loggable("ESP32_SMA_MQTT") {}

    // Destructor (optional, as the singleton instance will be destroyed when the program ends).
    ~ESP32_SMA_MQTT() {}



};


extern void E_formPage();
extern void E_connectAP();
extern void E_handleForm();


#endif