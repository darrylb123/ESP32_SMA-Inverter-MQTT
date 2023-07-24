
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include "ESP32Loggable.h"

#include <WiFi.h>
#include <WebServer.h>
// #include <DNSServer.h>
// #include <ESPmDNS.h>
#include <Preferences.h>
#include <PubSubClient.h>

#include "ESP32_SMA_Inverter_App_Config.h"
#include "config_values.h"


#define FORMAT_LITTLEFS_IF_FAILED true

// Our configuration structure.
//
// Never use a JsonDocument to store the configuration!
// A JsonDocument is *not* a permanent storage; it's only a temporary storage
// used during the serialization phase. See:
// https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/



// Loads the configuration from a file
void ESP32_SMA_Inverter_App_Config::loadConfiguration() {
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
   String arr[] = {"mqttBroker", "mqttPort", "mqttUser","mqttPasswd", "mqttTopic","smaInvPass", "smaBTAddress", "scanRate", "hassDisc"};
   for (int i=0;i<arr->length();i++) {
    String k = arr[i];
    String v = doc[k];
    log_w("load key: %s , value: %s", k, v);
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
  #endif

  
  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
  
}

// Saves the configuration to a file
void ESP32_SMA_Inverter_App_Config::saveConfiguration() {
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

  String arr[] = {"mqttBroker", "mqttPort", "mqttUser","mqttPasswd", "mqttTopic","smaInvPass", "smaBTAddress", "scanRate", "hassDisc"};
  
   for (int i=0;i<arr->length();i++) {
    String k = arr[i];
    String v = doc[k];
    log_w("save key: %s , value: %s", k, v);
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
void ESP32_SMA_Inverter_App_Config::printFile() {
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

void ESP32_SMA_Inverter_App_Config::configSetup() {
  
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

void ESP32_SMA_Inverter_App_Config::rmfiles(){
  if (LittleFS.remove("/config.txt")) {
    log_w("%s removed", "/config.txt");
  } else {
    log_e("%s removal failed", "/config.txt");
  }
}
