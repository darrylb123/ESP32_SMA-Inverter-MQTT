#pragma once 
#ifndef ESP32_SMA_MQTT_H
#define ESP32_SMA_MQTT_H


#include <WiFi.h>

#include <Preferences.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <WebServer.h>


class ESP32_SMA_MQTT {

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
    static void wifiStartup();
    static void mySmartConfig();
    static void connectAP();
    static void wifiLoop();
    static void formPage ();
    static void handleForm();
    static void brokerConnect();
    static bool publishData();
    static void hassAutoDiscover();
    static void sendLongMQTT(char *topic, char *postscript, char *msg);
    static void logViaMQTT(char *logStr);

private:
    // Private constructor to prevent instantiation from outside the class.
    ESP32_SMA_MQTT() {}

    // Destructor (optional, as the singleton instance will be destroyed when the program ends).
    ~ESP32_SMA_MQTT() {}


    protected:        
        char sapString[21];
        unsigned long previousMillis = 0;
        unsigned long interval = 30000;

};





#endif