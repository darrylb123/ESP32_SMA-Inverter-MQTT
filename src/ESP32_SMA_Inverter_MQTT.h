#pragma once 
#ifndef ESP32_SMA_INVERTER_APP_H
#define ESP32_SMA_INVERTER_APP_H

#include <WiFiClient.h>
#include <WebServer.h>
#include <PubSubClient.h>

#include "ESP32_SMA_Inverter_App_Config.h"
#include "SMA_Inverter.h"
#include "ESP32Loggable.h"


class ESP32_SMA_Inverter_App : public ESP32Loggable {

    public:
        ESP32_SMA_Inverter_App() :  ESP32Loggable("ESP32_SMA_Inverter_App") {
            config = AppConfig();
            strcpy(smaInvPass, "0000");
            for (int i=0;i<6;i++) {
                smaBTAddress[i]='0';
            }
        };

    void appSetup();
    void appLoop();
    void wifiStartup();
    void logBuild();

    static const uint16_t appSUSyID = 125;
    static uint32_t appSerial ;

    static int smartConfig;

    static WebServer webServer;
    static WiFiClient espClient;
    static PubSubClient client;

    protected:
      //extern BluetoothSerial serialBT;
        bool nightTime = false;
        bool firstTime = true;
    private: 

        AppConfig config;
        char smaInvPass[12];  
        uint8_t smaBTAddress[6]; // SMA bluetooth address
        //uint8_t  espBTAddress[6]; // is retrieved from BT packet

        uint32_t nextTime = 0;


};


#endif