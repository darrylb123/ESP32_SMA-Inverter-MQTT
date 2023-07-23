#ifndef MQTT.H
#define MQTT.H
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>












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
void sendLongMQTT(char *msg);
void logViaMQTT(char *logStr);
#endif