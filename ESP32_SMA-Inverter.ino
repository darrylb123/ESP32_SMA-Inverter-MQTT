/* MIT License

Copyright (c) 2022 Lupo135

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//*** debug ****************
// 0=none; 
// 1=Values only; 
// 2=Values and Info and P-buffer
// 3=Values and Info and T+R+P-buffer
#define DEBUG_SMA 1

#include <Esp.h>
#include "Arduino.h"
#include "BluetoothSerial.h"
#include "SMA_Inverter.h"

#include "Utils.h"

const uint16_t AppSUSyID = 125;
uint32_t  AppSerial;
//
#define maxpcktBufsize 256
uint8_t  BTrdBuf[maxpcktBufsize];    //BT read buffer
uint8_t  pcktBuf[maxpcktBufsize];
uint16_t pcktBufPos = 0;
uint16_t pcktBufMax = 0; // max. used size of PcktBuf
uint16_t pcktID = 1;
const char BTPin[] = {'0','0','0','0',0}; // BT pin Always 0000. (not login passcode!)

#define USERGROUP UG_USER
// SMA login password for UG_USER or UG_INSTALLER always 12 char. Unused=0x00
const char SmaInvPass[]={'P','a','s','s','w','o','r','d',0,0,0,0}; 
uint8_t SmaBTAddress[6]  = {0x00, 0x80, 0x25, 0x29, 0xEB, 0xD3}; // my SMA SMC6000TL 00:80:25:29:eb:d3 
uint8_t EspBTAddress[6]; // is retrieved from BT packet

InverterData InvData;
InverterData *pInvData = &InvData;

BluetoothSerial SerialBT;


void setup() { 
  Serial.begin(115200);          //Serial port for debugging output
  delay(3000);
  pInvData->SUSyID = 0x7d;
  pInvData->Serial = 0;
  for(uint8_t i=0; i<6; i++) pInvData->BTAddress[i] = SmaBTAddress[5-i]; // reverse order
  DEBUG2_PRINTF("pInvData->BTAddress: %02X:%02X:%02X:%02X:%02X:%02X\n",
               pInvData->BTAddress[5], pInvData->BTAddress[4], pInvData->BTAddress[3],
               pInvData->BTAddress[2], pInvData->BTAddress[1], pInvData->BTAddress[0]);
  SerialBT.begin("ESP32test", true);   // "true" creates this device as a BT Master.
  SerialBT.setPin(&BTPin[0]); 
} 

void loop() { 
  // **** Connect SMA **********
  DEBUG1_PRINTLN("Connecting to SMA inverter.");
  while (1) {
    if (SerialBT.connect(SmaBTAddress)) {
      break;
    } else {
        DEBUG1_PRINT("."); 
        delay(20000);
    }
  }
  // **** Initialize SMA *******
  DEBUG1_PRINTLN("*** Initialize");
  E_RC rc = initialiseSMAConnection();

  getBT_SignalStrength();
  
  // **** logon SMA ************
  DEBUG1_PRINTLN("\n*** logonSMAInverter");
  rc = logonSMAInverter(SmaInvPass, USERGROUP);

  if ((getInverterData(OperationTime)) != 0)     DEBUG1_PRINTLN("OperationTime error!"    ); // OperTime + OperTime
  delay(100);
  #define LOOPTIME_SEC 20
  uint32_t nextTime = 0;
  while (1) {
    if (nextTime<millis()) nextTime = LOOPTIME_SEC*1000+millis();

    if ((getInverterData(SpotGridFrequency)) != 0) DEBUG1_PRINTLN("SpotGridFrequency error!");
  //if ((getInverterData(SpotDCPower)) != 0)       DEBUG1_PRINTLN("getSpotDCPower error!"   ); //pcktBuf[23]=15 error!
  //if ((getInverterData(SpotACPower)) != 0)       DEBUG1_PRINTLN("SpotACPower error!"      ); //pcktBuf[23]=15 error!
    if ((getInverterData(SpotACVoltage)) != 0)     DEBUG1_PRINTLN("getSpotACVoltage error!" ); // Uac + Iac
    if ((getInverterData(SpotACTotalPower)) != 0)  DEBUG1_PRINTLN("SpotACTotalPower error!" ); // Pac
    if ((getInverterData(EnergyProduction)) != 0)  DEBUG1_PRINTLN("EnergyProduction error!" ); // E-Total + E-Today
    if ((getInverterData(SpotDCVoltage)) != 0)     DEBUG1_PRINTLN("getSpotDCVoltage error!" ); // Udc + Idc
  //if ((getInverterData(InverterTemp)) != 0)      DEBUG1_PRINTLN("InverterTemp error!"     ); //pcktBuf[23]=15 error!

  // **** calculate efficiency *
    DEBUG1_PRINTF("\nEfficiency %8.2f %%  ", (double)pInvData->Uac * pInvData->Iac / (double)(pInvData->Udc * pInvData->Idc ) * 100);
    DEBUG1_PRINT("\n *************************"   );

  // **** delay ****************
    //delay(20000);
    do { delay(500); } while (nextTime>millis());
    nextTime += LOOPTIME_SEC*1000;
  }
}
