#ifndef CONFIG.H
#define CONFIG.H

#include <Arduino.h>
#include "config_values.h"

//*** debug ****************
// 0=no Debug; 
// 1=values only; 
// 2=values and info and P-buffer
// 3=values and info and T+R+P-buffer
#define DEBUG_SMA 1



// SMA login password for UG_USER or UG_INSTALLER always 12 char. Unused=0x00
#define USERGROUP UG_USER

*/
// Configuration struct

// Uncomment to logoff the inverter after each connection
// Helps with connection reliabiolity on some inverters
// #define LOGOFF true


#include <WiFi.h>
#include <WebServer.h>
// #include <DNSServer.h>
// #include <ESPmDNS.h>
#include <Preferences.h>
#include <PubSubClient.h>

struct Config {
  String mqttBroker;
  uint16_t mqttPort;
  String mqttUser;
  String mqttPasswd;
  String mqttTopic;
  String SmaInvPass;
  String SmaBTAddress;
  int ScanRate;
  bool hassDisc;
};

const char *confFile = "/config.txt";  

//Prototypes
void loadConfiguration(const char *filename, Config &config);
void saveConfiguration(const char *confFile, const Config &config);
void printFile(const char *confFile);
void configSetup();
void rmfiles();
#endif