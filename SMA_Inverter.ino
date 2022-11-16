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


#include <Esp.h>
#include "Arduino.h"
#include "BluetoothSerial.h"
#include "SMA_Inverter.h"
#include "SMA_bluetooth.h"



int32_t value32 = 0;
int64_t value64 = 0;

extern bool ReadTimeout;
extern uint16_t FCSChecksum;

bool isValidSender(uint8_t expAddr[6], uint8_t isAddr[6]) {
  for (int i = 0; i < 6; i++)
    if ((isAddr[i] != expAddr[i]) && (expAddr[i] != 0xFF)) {
      DEBUG2_PRINTF("\nShoud-Addr: %02X %02X %02X %02X %02X %02X\n   Is-Addr: %02X %02X %02X %02X %02X %02X\n",
        expAddr[5], expAddr[4], expAddr[3], expAddr[2], expAddr[1], expAddr[5],
         isAddr[5],  isAddr[4],  isAddr[3],  isAddr[2],  isAddr[1],  isAddr[5]);
        return false;
    }
  return true;
}


// ----------------------------------------------------------------------------------------------
//unsigned int readBtPacket(int index, unsigned int cmdcodetowait) {
E_RC getPacket(uint8_t expAddr[6], int wait4Command) {
  DEBUG2_PRINTF("\ngetPacket command=0x%04x\n", wait4Command);
  int index = 0;
  bool hasL2pckt = false;
  E_RC rc = E_OK; 
  L1Hdr *pL1Hdr = (L1Hdr *)&BTrdBuf[0];
  do {
    // read L1Hdr
    uint8_t rdCnt=0;
    for (rdCnt=0;rdCnt<18;rdCnt++) {
      BTrdBuf[rdCnt]= BTgetByte();
      if (ReadTimeout)  break;
    }
    if (rdCnt == 0) return E_NODATA;
    // Validate L1 header
   //if (!((BTrdBuf[0] ^ BTrdBuf[1] ^ BTrdBuf[2]) == BTrdBuf[3])) {
   //  DEBUG1_PRINT("\nWrong L1 CRC!!" );
   //#if (DEBUG_SMA > 2)
   //  HexDump(BTrdBuf, rdCnt, 10, 'R');
   //#endif
   //  return 1;
   //}

    DEBUG2_PRINTF("L1 Rec=%d bytes pkL=0x%04x=%d Cmd=0x%04x\n",
        rdCnt, pL1Hdr->pkLength, pL1Hdr->pkLength, pL1Hdr->command);

    if (pL1Hdr->pkLength > sizeof(L1Hdr)) { // more bytes to read
      for (rdCnt=18; rdCnt<pL1Hdr->pkLength; rdCnt++) {
        BTrdBuf[rdCnt]= BTgetByte();
        if (ReadTimeout) break;
      }
      DEBUG3_PRINTF("L2 Rec=%d bytes\n", rdCnt);
      #if (DEBUG_SMA > 2)
      HexDump(BTrdBuf, rdCnt, 10, 'R');
      #endif

      //Check if data is coming from the right inverter
      if (isValidSender(expAddr, pL1Hdr->SourceAddr)) {
        rc = E_OK;

        DEBUG2_PRINTF("\nHasL2pckt: 0x7E?=0x%02X 0x656003FF?=0x%08X\n", BTrdBuf[18], get_u32(BTrdBuf+19));
        if ((hasL2pckt == 0) && (BTrdBuf[18] == 0x7E) && (get_u32(BTrdBuf+19) == 0x656003FF)) {
          hasL2pckt = true;
        }

        if (hasL2pckt) {
          //Copy BTrdBuf to pcktBuf
          bool escNext = false;
          DEBUG2_PRINTF("PacketLength=%d\n", pL1Hdr->pkLength);

          for (int i=sizeof(L1Hdr); i<pL1Hdr->pkLength; i++) {
            pcktBuf[index] = BTrdBuf[i];
            //Keep 1st byte raw unescaped 0x7E
            if (escNext == true) {
              pcktBuf[index] ^= 0x20;
              escNext = false;
              index++;
            } else {
              if (pcktBuf[index] == 0x7D)
                escNext = true; //Throw away the 0x7d byte
              else
                index++;
            }
            if (index >= maxpcktBufsize) {
              DEBUG1_PRINTF("pcktBuf overflow! (%d)\n", index);
            }
          }
          pcktBufPos = index;
        } else {  // no L2pckt
          memcpy(pcktBuf, BTrdBuf, rdCnt);
          pcktBufPos = rdCnt;
        }
      } else { // isValidSender()
          rc = E_RETRY;
      }
    } else {  // L1 only
    #if (DEBUG_SMA > 2)
      HexDump(BTrdBuf, rdCnt, 10, 'R');
    #endif
      //Check if data is coming from the right inverter
      if (isValidSender(expAddr, pL1Hdr->SourceAddr)) {
          rc = E_OK;

          memcpy(pcktBuf, BTrdBuf, rdCnt);
          pcktBufPos = rdCnt;
      } else { // isValidSender()
          rc = E_RETRY;
      }
    }
    if (BTrdBuf[0] != '\x7e') { 
       SerialBT.flush();
       DEBUG3_PRINT("\nCommBuf[0]!=0x7e -> BT-flush");
    }
  } while (((pL1Hdr->command != wait4Command) || (rc == E_RETRY)) && (0xFF != wait4Command));

  if ((rc == E_OK) ) {
  #if (DEBUG_SMA > 1)
    DEBUG2_PRINT("<<<====Rd Content of pcktBuf =======>>>");
    HexDump(pcktBuf, pcktBufPos, 10, 'P');
    DEBUG2_PRINT("==>>>");
  #endif
  }

  if (pcktBufPos > pcktBufMax) {
    pcktBufMax = pcktBufPos;
    DEBUG2_PRINTF("pcktBufMax is now %d bytes\n", pcktBufMax);
  }

  return rc;
}

// *************************************************
void writePacketHeader(uint8_t *buf, const uint16_t control, const uint8_t *destaddress) {
    pcktBufPos = 0;

    FCSChecksum = 0xFFFF;
    buf[pcktBufPos++] = 0x7E;
    buf[pcktBufPos++] = 0;  //placeholder for len1
    buf[pcktBufPos++] = 0;  //placeholder for len2
    buf[pcktBufPos++] = 0;  //placeholder for checksum
    int i;
    for(i = 0; i < 6; i++) buf[pcktBufPos++] = EspBTAddress[i];
    for(i = 0; i < 6; i++) buf[pcktBufPos++] = destaddress[i];

    buf[pcktBufPos++] = (uint8_t)(control & 0xFF);
    buf[pcktBufPos++] = (uint8_t)(control >> 8);
}
// ***********************************************
E_RC getInverterDataCfl(uint32_t command, uint32_t first, uint32_t last) {
    pcktID++;
    writePacketHeader(pcktBuf, 0x01, sixff); //addr_unknown);
    //if (pInvData->SUSyID == SID_SB240)
    //writePacket(pcktBuf, 0x09, 0xE0, 0, pInvData->SUSyID, pInvData->Serial);
    //else
    writePacket(pcktBuf, 0x09, 0xA0, 0, pInvData->SUSyID, pInvData->Serial);
    write32(pcktBuf, command);
    write32(pcktBuf, first);
    write32(pcktBuf, last);
    writePacketTrailer(pcktBuf);
    writePacketLength(pcktBuf);

    BTsendPacket(pcktBuf);

    uint8_t pcktcount = 0;
    bool  validPcktID = false;
    do {
     do {
      pInvData->status = getPacket(pInvData->BTAddress, 0x0001);
   
      if (pInvData->status != E_OK) return pInvData->status;
      if (!validateChecksum()) {
        pInvData->status = E_CHKSUM;
        return pInvData->status;
      } else { // packet complete -> check content
        if ((pInvData->status = (E_RC)get_u16(pcktBuf + 23)) != E_OK) {
          DEBUG2_PRINTF("Packet status: 0x%02X\n", pInvData->status);
          return pInvData->status;
        }
      // *** analyze received data ***
        pcktcount = get_u16(pcktBuf + 25);
        uint8_t rcvpcktID = get_u16(pcktBuf + 27) & 0x7FFF;
        if (pcktID == rcvpcktID) {
          if ((get_u16(pcktBuf + 15) == pInvData->SUSyID) 
            && (get_u32(pcktBuf + 17) == pInvData->Serial)) {
            validPcktID = true;
            value32 = 0;
            value64 = 0;
            uint16_t recordsize = 4 * ((uint32_t)pcktBuf[5] - 9) / (get_u32(pcktBuf + 37) - get_u32(pcktBuf + 33) + 1);
            DEBUG2_PRINTF("\npcktID=0x%04x recordsize=%d pcktBufPos=%d pcktcount=%04x", 
                                           recordsize, pcktBufPos, rcvpcktID, pcktcount);
            for (uint16_t ii = 41; ii < pcktBufPos - 3; ii += recordsize) {
              uint8_t *recptr = pcktBuf + ii;
              uint32_t code = get_u32(recptr);
              //LriDef lri = (LriDef)(code & 0x00FFFF00);
              uint16_t lri = (code & 0x00FFFF00) >> 8;
              uint32_t cls = code & 0xFF;
              uint8_t dataType = code >> 24;
              time_t datetime = (time_t)get_u32(recptr + 4);
              DEBUG3_PRINTF("\nlri=0x%04x cls=0x%08X dataType=0x%02x",lri, cls, dataType);
       
              if (recordsize == 16) {
                value64 = get_u64(recptr + 8);
                DEBUG2_PRINTF("\nvalue64=%d=0x%016x",value64, value64);
       
                  //if (is_NaN(value64) || is_NaN((uint64_t)value64)) value64 = 0;
              } else if ((dataType != 16) && (dataType != 8)) { // ((dataType != DT_STRING) && (dataType != DT_STATUS)) {
                value32 = get_u32(recptr + 16);
                DEBUG3_PRINTF("\nvalue32=%d=0x%08x",value32, value32);
              }
              switch (lri) {
              case GridMsTotW: //SPOT_PACTOT
                  //This function gives us the time when the inverter was switched off
                  //pInvData->SleepTime = datetime;
                  pInvData->Pac = value32;
                  //debug_watt("SPOT_PACTOT", value32, datetime);
                  DEBUG1_PRINTF("\nPac %15.3f kW ", tokW(value32));
                  printUnixTime(datetime);
                  break;
       
              case GridMsWphsA: //SPOT_PAC1
                  pInvData->Pmax = value32;
                  //debug_watt("SPOT_PAC1", value32, datetime);
                  DEBUG1_PRINTF("\nPmax %14.2f kW ", tokW(value32));
                  printUnixTime(datetime);
                  break;
       
              case GridMsPhVphsA: //SPOT_UAC1
                  pInvData->Uac = value32;
                  //debug_volt("SPOT_UAC1", value32, datetime);
                  DEBUG1_PRINTF("\nUac %15.2f V  ", toVolt(value32));
                  printUnixTime(datetime);
                  break;
       
              case GridMsAphsA_1: //SPOT_IAC1
              case GridMsAphsA:
                  pInvData->Iac = value32;
                  //debug_amp("SPOT_IAC1", value32, datetime);
                  DEBUG1_PRINTF("\nIac %15.2f A  ", toAmp(value32));
                  printUnixTime(datetime);
                  break;
       
              case GridMsHz: //SPOT_FREQ
                  //pInvData->GridFreq = value32;
                  DEBUG1_PRINTF("\nFreq %14.2f Hz ", toHz(value32));
                  printUnixTime(datetime);
                  break;
       
              case DcMsWatt: //SPOT_PDC1 / SPOT_PDC2
                  DEBUG1_PRINTF("\nPDC %15.2f kW ", tokW(value32));
                  printUnixTime(datetime);
                  break;
       
              case DcMsVol: //SPOT_UDC1 / SPOT_UDC2
                  pInvData->Udc = value32;
                  DEBUG1_PRINTF("\nUdc %15.2f V  ", toVolt(value32));
                  printUnixTime(datetime);
                  break;
       
              case DcMsAmp: //SPOT_IDC1 / SPOT_IDC2
                  pInvData->Idc = value32;
                  DEBUG1_PRINTF("\nIdc %15.2f A  ", toAmp(value32));
                  printUnixTime(datetime);
                  break;
       
              case MeteringTotWhOut: //SPOT_ETOTAL
                  //In case SPOT_ETODAY missing, this function gives us inverter time (eg: SUNNY TRIPOWER 6.0)
                  //pInvData->InverterDatetime = datetime;
                  pInvData->ETotal = value64;
                  //debug_kwh("SPOT_ETOTAL", value64, datetime);
                  DEBUG1_PRINTF("\nE-Total %11.3f kWh", tokWh(value64));
                  printUnixTime(datetime);
                  break;
       
              case MeteringDyWhOut: //SPOT_ETODAY
                  //This function gives us the current inverter time
                  //pInvData->InverterDatetime = datetime;
                  pInvData->EToday = value64;
                  //debug_kwh("SPOT_ETODAY", value64, datetime);
                  DEBUG1_PRINTF("\nE-Today %11.3f kWh", tokWh(value64));
                  printUnixTime(datetime);
                  break;
       
              case MeteringTotOpTms: //SPOT_OPERTM
                  pInvData->OperationTime = value64;
                  //debug_hour("SPOT_OPERTM", value64, datetime);
                  DEBUG1_PRINTF("\nOperTime  %7.3f h  ", toHour(value64));
                  printUnixTime(datetime);
                  break;
       
              case MeteringTotFeedTms: //SPOT_FEEDTM
                  pInvData->FeedInTime = value64;
                  //debug_hour("SPOT_FEEDTM", value64, datetime);
                  DEBUG1_PRINTF("\nFeedTime  %7.3f h  ", toHour(value64));
                  printUnixTime(datetime);
                  break;
       
              case CoolsysTmpNom:
                  //pInvData->Temperature = value32;
                  DEBUG1_PRINTF("\nTemp.     %7.3f �C ", toTemp(value32));
                  break;
       
              case MeteringGridMsTotWOut:
                  //pInvData->MeteringGridMsTotWOut = value32;
                  break;
       
              case MeteringGridMsTotWIn:
                  //pInvData->MeteringGridMsTotWIn = value32;
                  break;
              }
            } //for
          } else {
            DEBUG3_PRINTF("*** Wrong SUSyID=%04x=%04x Serial=%08x=%08x", 
                 get_u16(pcktBuf + 15), pInvData->SUSyID, get_u32(pcktBuf + 17),pInvData->Serial);
          }
        } else {
          DEBUG3_PRINTF("Packet ID mismatch: exp=0x%02X is=0x%02X\n", pcktID, rcvpcktID);
          validPcktID = false;
          pcktcount = 0;
        }
      } // valid checksum
    } while (pcktcount > 0);
   } while (!validPcktID);
   return pInvData->status;
}
// ***********************************************
E_RC getInverterData(enum getInverterDataType type) {
  E_RC rc = E_OK;
  uint32_t command;
  uint32_t first;
  uint32_t last;

  switch (type) {
  case EnergyProduction:
      DEBUG2_PRINT("\n*** EnergyProduction ***");
      // SPOT_ETODAY, SPOT_ETOTAL
      command = 0x54000200;
      first = 0x00260100;
      last = 0x002622FF;
      break;

  case SpotDCPower:
      DEBUG2_PRINT("\n*** SpotDCPower ***");
      // SPOT_PDC1, SPOT_PDC2
      command = 0x53800200;
      first = 0x00251E00;
      last = 0x00251EFF;
      break;

  case SpotDCVoltage:
      DEBUG2_PRINT("\n*** SpotDCVoltage ***");
      // SPOT_UDC1, SPOT_UDC2, SPOT_IDC1, SPOT_IDC2
      command = 0x53800200;
      first = 0x00451F00;
      last = 0x004521FF;
      break;

  case SpotACPower:
      DEBUG2_PRINT("\n*** SpotACPower ***");
      // SPOT_PAC1, SPOT_PAC2, SPOT_PAC3
      command = 0x51000200;
      first = 0x00464000;
      last = 0x004642FF;
      break;

  case SpotACVoltage:
      DEBUG2_PRINT("\n*** SpotACVoltage ***");
      // SPOT_UAC1, SPOT_UAC2, SPOT_UAC3, SPOT_IAC1, SPOT_IAC2, SPOT_IAC3
      command = 0x51000200;
      first = 0x00464800;
      last = 0x004655FF;
      break;

  case SpotGridFrequency:
      DEBUG2_PRINT("\n*** SpotGridFrequency ***");
      // SPOT_FREQ
      command = 0x51000200;
      first = 0x00465700;
      last = 0x004657FF;
      break;

  case SpotACTotalPower:
      DEBUG2_PRINT("\n*** SpotACTotalPower ***");
      // SPOT_PACTOT
      command = 0x51000200;
      first = 0x00263F00;
      last = 0x00263FFF;
      break;

  case TypeLabel:
      DEBUG2_PRINT("\n*** TypeLabel ***");
      // INV_NAME, INV_TYPE, INV_CLASS
      command = 0x58000200;
      first = 0x00821E00;
      last = 0x008220FF;
      break;

  case SoftwareVersion:
      DEBUG2_PRINT("\n*** SoftwareVersion ***");
      // INV_SWVERSION
      command = 0x58000200;
      first = 0x00823400;
      last = 0x008234FF;
      break;

  case DeviceStatus:
      DEBUG2_PRINT("\n*** DeviceStatus ***");
      // INV_STATUS
      command = 0x51800200;
      first = 0x00214800;
      last = 0x002148FF;
      break;

  case GridRelayStatus:
      DEBUG2_PRINT("\n*** GridRelayStatus ***");
      // INV_GRIDRELAY
      command = 0x51800200;
      first = 0x00416400;
      last = 0x004164FF;
      break;

  case OperationTime:
      DEBUG2_PRINT("\n*** OperationTime ***");
      // SPOT_OPERTM, SPOT_FEEDTM
      command = 0x54000200;
      first = 0x00462E00;
      last = 0x00462FFF;
      break;

  case InverterTemp:
      DEBUG2_PRINT("\n*** InverterTemp ***");
      command = 0x52000200;
      first = 0x00237700;
      last = 0x002377FF;
      break;

  case MeteringGridMsTotW:
      DEBUG2_PRINT("\n*** MeteringGridMsTotW ***");
      command = 0x51000200;
      first = 0x00463600;
      last = 0x004637FF;
      break;

  default:
      DEBUG1_PRINT("\nInvalid getInverterDataType!!");
      return E_BADARG;
  };

  // Request data from inverter
  for (uint8_t retries=1;; retries++) {
    rc = getInverterDataCfl(command, first, last);
    if (rc != E_OK) {
      if (retries>5) {
         return rc;
      }
      DEBUG2_PRINTF("\nRetrying.%d",retries);
    } else {
      break;
    } 
  }

  return rc;
}

//-------------------------------------------------------------------------
bool getBT_SignalStrength() {
  DEBUG2_PRINT("\n\n*** SignalStrength ***");
  writePacketHeader(pcktBuf, 0x03, pInvData->BTAddress);
  writeByte(pcktBuf,0x05);
  writeByte(pcktBuf,0x00);
  writePacketLength(pcktBuf);
  BTsendPacket(pcktBuf);

  getPacket(pInvData->BTAddress, 4);
  DEBUG1_PRINTF("BT-Signal %9.1f %%", ((float)BTrdBuf[22] * 100.0f / 255.0f));
  return true;
}
//-------------------------------------------------------------------------
E_RC initialiseSMAConnection() {
  DEBUG2_PRINTLN("Connected succesfully! -> Initialize");
  getPacket(pInvData->BTAddress, 2); // 1. Receive
  pInvData->NetID = pcktBuf[22];
  DEBUG2_PRINTF("SMA netID=%02X\n", pInvData->NetID);
  writePacketHeader(pcktBuf, 0x02, pInvData->BTAddress);
  write32(pcktBuf, 0x00700400);
  writeByte(pcktBuf, pInvData->NetID);
  write32(pcktBuf, 0);
  write32(pcktBuf, 1);
  writePacketLength(pcktBuf);

  BTsendPacket(pcktBuf);             // 1. Reply
  getPacket(pInvData->BTAddress, 5); // 2. Receive

  // Extract ESP32 BT address
  memcpy(EspBTAddress, pcktBuf+26,6); 
  DEBUG3_PRINTF("ESP32 BT address: %02X:%02X:%02X:%02X:%02X:%02X\n",
             EspBTAddress[5], EspBTAddress[4], EspBTAddress[3],
             EspBTAddress[2], EspBTAddress[1], EspBTAddress[0]);

  pcktID++;
  writePacketHeader(pcktBuf, 0x01, sixff); //addr_unknown);
  writePacket(pcktBuf, 0x09, 0xA0, 0, 0xFFFF, 0xFFFFFFFF); // anySUSyID, anySerial);
  write32(pcktBuf, 0x00000200);
  write32(pcktBuf, 0);
  write32(pcktBuf, 0);
  writePacketTrailer(pcktBuf);
  writePacketLength(pcktBuf);

  BTsendPacket(pcktBuf);             // 2. Reply
  if (getPacket(pInvData->BTAddress, 1) != E_OK) // 3. Receive
    return E_INIT;

  if (!validateChecksum())
    return E_CHKSUM;

  pInvData->Serial = get_u32(pcktBuf + 57);
  DEBUG1_PRINTF("Serial Nr: %lu\n", pInvData->Serial);
  return E_OK;
}
// **** Logon SMA **********
E_RC logonSMAInverter(const char *password, const uint8_t user) {
  #define MAX_PWLENGTH 12
  uint8_t pw[MAX_PWLENGTH];
  E_RC rc = E_OK;

  // Encode password
  uint8_t encChar = (user == USERGROUP)? 0x88:0xBB;
  uint8_t idx;
  for (idx = 0; (password[idx] != 0) && (idx < sizeof(pw)); idx++)
    pw[idx] = password[idx] + encChar;
  for (; idx < MAX_PWLENGTH; idx++) pw[idx] = encChar;

    bool validPcktID = false;
    time_t now;
    pcktID++;
    now = time(NULL);
    writePacketHeader(pcktBuf, 0x01, sixff);
    writePacket(pcktBuf, 0x0E, 0xA0, 0x0100, 0xFFFF, 0xFFFFFFFF); // anySUSyID, anySerial);
    write32(pcktBuf, 0xFFFD040C);
    write32(pcktBuf, user); //userGroup);    // User / Installer
    write32(pcktBuf, 0x00000384); // Timeout = 900sec ?
    write32(pcktBuf, now);
    write32(pcktBuf, 0);
    writeArray(pcktBuf, pw, sizeof(pw));
    writePacketTrailer(pcktBuf);
    writePacketLength(pcktBuf);

    BTsendPacket(pcktBuf);
    if ((rc = getPacket(sixff, 1)) != E_OK) return rc;

    if (!validateChecksum()) return E_CHKSUM;
    uint8_t rcvpcktID = get_u16(pcktBuf+27) & 0x7FFF;
    if ((pcktID == rcvpcktID) && (get_u32(pcktBuf + 41) == now)) {
      pInvData->SUSyID = get_u16(pcktBuf + 15);
      pInvData->Serial = get_u32(pcktBuf + 17);
      DEBUG3_PRINTF("Set:->SUSyID=0x%02X ->Serial=0x%02X ", pInvData->SUSyID, pInvData->Serial);
      validPcktID = true;
      uint8_t retcode = get_u16(pcktBuf + 23);
      // switch (retcode) {
      //     case 0: rc = E_OK; break;
      //     case 0x0100: rc = E_INVPASSW; break;
      //     default: rc = (E_SBFSPOT)retcode; break;
      // }
    } else { 
      DEBUG1_PRINTF("Unexpected response  %02X:%02X:%02X:%02X:%02X:%02X pcktID=0x%04X rcvpcktID=0x%04X now=0x%04X", 
                   BTrdBuf[9], BTrdBuf[8], BTrdBuf[7], BTrdBuf[6], BTrdBuf[5], BTrdBuf[4],
                   pcktID, rcvpcktID, now);
      rc = E_INVRESP;
    }
    return rc;
}
