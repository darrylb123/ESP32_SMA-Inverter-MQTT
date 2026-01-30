#ifndef SMA_WIFI_CONFIG_VALUES_H
#define SMA_WIFI_CONFIG_VALUES_H


//*** debug ****************
// 0=no Debug; 
// 1=values only; 
// 2=values and info and P-buffer
// 3=values and info and T+R+P-buffer
#define DEBUG_SMA 1

#define LOOPTIME_SEC 30

// ------------------------------------------------------------------------------------------------
// All the following configuration can be configured after flashing if desired
// Install the ESPTouch app on your smart phone to configure the Wifi Credentials
// Your smart phone must be connected to the Wifi before starting the ESPTouch app
// Then use a browser to configure the other parameters once the device is connected to the network
// ------------------------------------------------------------------------------------------------



// SMA login password for UG_USER or UG_INSTALLER always 12 char. Unused=0x00
// #define USERGROUP UG_USER

// #define WIFI_SSID "ssid"
// #define WIFI_PASSWORD "ssidpassword"

// #define MQTT_BROKER "192.168.0.100"
// #define MQTT_PORT  1883
// #define MQTT_USER  "mqttuser"
// #define MQTT_PASS  "mqttpass"
// #define MQTT_topic "SMA"
// #define SMA_PASS  "inverterpass"
// #define SMA_BTADDRESS "00:80:25:00:00:00"
// #define SCAN_RATE  60
// #define HASS_DISCOVERY false
// #define HASS_STATE_TOPIC "homeassistant/sensor/%s/state"
// #define TIMEZONE 1
// #define NTPHOSTNAME "pool.ntp.org"
// #define SUNUP 6
// #define SUNDOWN 18
// #define NIGHTSCANRATE 15*60*1000
// #define THISSERIAL 0



#endif
