#pragma once 
#ifndef ESP32_SMA_INVERTER_CONFIG_H
#define ESP32_SMA_INVERTER_CONFIG_H

#include "ESP32Loggable.h"

//#include "config_values.h"

//*** debug ****************
// 0=no Debug; 
// 1=values only; 
// 2=values and info and P-buffer
// 3=values and info and T+R+P-buffer
#define DEBUG_SMA 1



// SMA login password for UG_USER or UG_INSTALLER always 12 char. Unused=0x00
#define USERGROUP UG_USER


//extern AppConfig config;
#define confFile "/config.txt" //extern const char *confFile = "/config.txt";  



// Configuration struct

// Uncomment to logoff the inverter after each connection
// Helps with connection reliabiolity on some inverters
// #define LOGOFF true

struct AppConfig {
    String mqttBroker;
    uint16_t mqttPort;
    String mqttUser;
    String mqttPasswd;
    String mqttTopic;
    String smaInvPass;
    String smaBTAddress;
    int scanRate;
    bool hassDisc;
};


class ESP32_SMA_Inverter_App_Config : public ESP32Loggable {

  public: 
    // Static method to get the instance of the class.
    static ESP32_SMA_Inverter_App_Config& getInstance() {
        // This guarantees that the instance is created only once.
        static ESP32_SMA_Inverter_App_Config instance;
        return instance;
    }

    // Delete the copy constructor and the assignment operator to prevent cloning.
    ESP32_SMA_Inverter_App_Config(const ESP32_SMA_Inverter_App_Config&) = delete;
    ESP32_SMA_Inverter_App_Config& operator=(const ESP32_SMA_Inverter_App_Config&) = delete;


  public: 

    //Prototypes
    //static void loadConfiguration(const char *filename, AppConfig &paramAppConfig);
    //static void saveConfiguration(const char *configFile, const AppConfig &paramAppConfig);
     void loadConfiguration(const char *filename);
     void saveConfiguration(const char *configFile);
     void printFile(const char *configFile);
     void configSetup();
     void rmfiles();

     AppConfig appConfig ;
     //AppConfig *pConfig = &appConfig;

  protected:
  

  private: 
      // Private constructor to prevent instantiation from outside the class.
    ESP32_SMA_Inverter_App_Config() :  ESP32Loggable("ESP32_SMA_Inverter_App_Config") {
      appConfig = AppConfig();
      //pConfig = &appConfig;

    }

    // Destructor (optional, as the singleton instance will be destroyed when the program ends).
    ~ESP32_SMA_Inverter_App_Config() {}

};


#endif