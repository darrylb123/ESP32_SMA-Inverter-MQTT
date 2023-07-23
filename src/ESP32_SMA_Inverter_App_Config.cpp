
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

#include <WiFi.h>
#include <WebServer.h>
// #include <DNSServer.h>
// #include <ESPmDNS.h>
#include <Preferences.h>
#include <PubSubClient.h>

#include "ESP32_SMA_Inverter_App_Config.h"



#define FORMAT_LITTLEFS_IF_FAILED true

// Our configuration structure.
//
// Never use a JsonDocument to store the configuration!
// A JsonDocument is *not* a permanent storage; it's only a temporary storage
// used during the serialization phase. See:
// https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/



// Loads the configuration from a file
void ESP32_SMA_Inverter_App_Config::loadConfiguration(const char *filename) {
  // Open file for reading
  File file = LittleFS.open(filename,"r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config         
  appConfig.mqttBroker =     doc["mqttBroker"] | "";
  appConfig.mqttPort = doc["mqttPort"] | 1883 ;
  appConfig.mqttUser = doc["mqttUser"] | "";
  appConfig.mqttPasswd = doc["mqttPasswd"] | "";
  appConfig.mqttTopic = doc["mqttTopic"] | "SMA";
  appConfig.smaInvPass = doc["smaInvPass"] | "password";
  appConfig.smaBTAddress = doc["smaBTAddress"] | "AA:BB:CC:DD:EE:FF";
  appConfig.scanRate = doc["scanRate"] | 60 ;
  appConfig.hassDisc = doc["hassDisc"] | true ;
  
  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
  
}

// Saves the configuration to a file
void ESP32_SMA_Inverter_App_Config::saveConfiguration(const char *configFile) {
  // Delete existing file, otherwise the configuration is appended to the file
  LittleFS.remove(configFile);

  // Open file for writing
  File file = LittleFS.open(configFile, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

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
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}

// Prints the content of a file to the Serial
void ESP32_SMA_Inverter_App_Config::printFile(const char *configFile) {
  // Open file for reading
  File file = LittleFS.open(configFile,"r");
  if (!file) {
    Serial.println(F("Failed to read file"));
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
  // Initialize serial port
  Serial.begin(115200);
  while (!Serial) continue;
  
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println(F("LittleFS mount failed"));
    return;
  }

  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  loadConfiguration(confFile);

  // Create configuration file
  Serial.println(F("Saving configuration..."));
  saveConfiguration(confFile);

  // Dump config file
 Serial.println(F("Print config file..."));
  printFile(confFile);
}

void ESP32_SMA_Inverter_App_Config::rmfiles(){
  if (LittleFS.remove("/config.txt")) {
    Serial.println("/config.txt removed");
  } else {
    Serial.println("/config.txt removal failed");
  }
}
