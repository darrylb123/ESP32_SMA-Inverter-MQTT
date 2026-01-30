#pragma once 
#ifndef ESP32_SMA_INVERTER_APP_H
#define ESP32_SMA_INVERTER_APP_H

#include <Arduino.h>
#include <Esp.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <PubSubClient.h>

#include <BluetoothSerial.h>

#include "ESP32_SMA_MQTT.h"
#include "ESP32_SMA_Inverter_App.h"
#include "config_values.h"


#include <logging.hpp>
#include <ets-appender.hpp>
#include <udp-appender.hpp>

#include "SMA_Utils.h"
#include "SMA_Inverter.h"
#include "ESP32Loggable.h"

//#include "config_values.h"


// Uncomment to logoff the inverter after each connection
// Helps with connection reliabiolity on some inverters
// #define LOGOFF true


//*** debug ****************
// 0=no Debug; 
// 1=values only; 
// 2=values and info and P-buffer
// 3=values and info and T+R+P-buffer
#define DEBUG_SMA 1



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
    String ntphostname;
    float timezone;
    uint32_t thisSerial;
};


class ESP32_SMA_Inverter_App : public ESP32Loggable {

    public:

        // Static method to get the instance of the class.
        static ESP32_SMA_Inverter_App& getInstance() {
            // This guarantees that the instance is created only once.
            static ESP32_SMA_Inverter_App instance;
            return instance;
        }

        // Delete the copy constructor and the assignment operator to prevent cloning.
        ESP32_SMA_Inverter_App(const ESP32_SMA_Inverter_App&) = delete;
        ESP32_SMA_Inverter_App& operator=(const ESP32_SMA_Inverter_App&) = delete;



    void appSetup();
    void appLoop();
 //   void wifiStartup();
    void logBuild();


    static int smartConfig;

    static WebServer webServer;
    static WiFiClient espClient;
    static PubSubClient client;

    AppConfig appConfig;

    //Prototypes
     void loadConfiguration();
     void saveConfiguration();
     void printFile();
     void configSetup();
     void rmfiles();

    protected:
      //extern BluetoothSerial serialBT;
        bool nightTime = false;
        bool firstTime = true;
        bool dayNight = false;

    private: 
        ESP32_SMA_Inverter_App() :  ESP32Loggable("ESP32_SMA_Inverter_App") {
            logger().setLevel(esp32m::Debug);

            appConfig = AppConfig();
            strcpy(smaInvPass, "0000");
            /*for (int i=0;i<6;i++) {
                smaBTAddress[i]='0';
            }*/
        };

        ~ESP32_SMA_Inverter_App() {}

        char smaInvPass[12];  
        uint8_t smaBTAddress[6]; // SMA bluetooth address
        //uint8_t  espBTAddress[6]; // is retrieved from BT packet

        uint32_t nextTime = 0;
        int failCount = 0;

        const String confFile = "/config.txt"; //extern const char *confFile = "/config.txt";  

};








#endif