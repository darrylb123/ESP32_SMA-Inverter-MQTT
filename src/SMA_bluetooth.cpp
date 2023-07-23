/* MIT License

Copyright (c) 2022 Lupo135
Copyright (c) 2023 darrylb123

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
#include <Arduino.h>
#include "SMA_bluetooth.h"
#include "Utils.h"



bool ReadTimeout = false;
uint16_t FCSChecksum=0xffff;
BluetoothSerial SerialBT;
uint8_t sixzeros[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t sixff[6]   = {0xff,0xff,0xff,0xff,0xff,0xff};

PROGMEM prog_uint16_t  fcstab[256]  = {
  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
  0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
  0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
  0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
  0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
  0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
  0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
  0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
  0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
  0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
  0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
  0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
  0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
  0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
  0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


// **** receive BT byte *******
uint8_t BTgetByte() {
  ReadTimeout = false;
  //Returns a single byte from the bluetooth stream (with error timeout/reset)
  uint32_t time = 20000+millis(); // 20 sec 
  uint8_t  rec = 0;  

  while (!SerialBT.available() ) {
    //delay(5);  //Wait for BT byte to arrive
    if (millis() > time) { 
      DEBUG2_PRINTLN("BTgetByte Timeout");
      ReadTimeout = true;
      break;
    }
  }

  if (!ReadTimeout) rec = SerialBT.read();
  return rec;
}
// **** transmit BT buffer ****
void BTsendPacket( uint8_t *btbuffer ) {
  //DEBUG2_PRINTLN();
  for(int i=0;i<pcktBufPos;i++) {
    #ifdef DebugBT
    if (i==0) DEBUG2_PRINT("*** sStart=");
    if (i==1) DEBUG2_PRINT(" len=");
    if (i==3) {
      if ((0x7e ^ *(btbuffer+1) ^ *(btbuffer+2)) == *(btbuffer+3)) 
        DEBUG2_PRINT(" checkOK=");
      else  DEBUG2_PRINT(" checkFalse!!=");
    }
    if (i==4)  DEBUG2_PRINTF("\nfrMac[%d]=",i);
    if (i==10) DEBUG2_PRINTF(" toMac[%d]=",i);
    if (i==16) DEBUG2_PRINTF(" Type[%d]=",i);
    if ((i==18)||(i==18+16)||(i==18+32)||(i==18+48)) DEBUG2_PRINTF("\nsDat[%d]=",i);
    DEBUG2_PRINTF("%02X,", *(btbuffer+i)); // Print out what we are sending, in hex, for inspection.
    #endif
    SerialBT.write( *(btbuffer+i) );  // Send to SMA via ESP32 bluetooth
  }
  HexDump(btbuffer, pcktBufPos, 10, 'T');
}


//-------------------------------------------------------------------------
// ********************************************************
void writeByte(uint8_t *btbuffer, uint8_t v) {
  //Keep a rolling checksum over the payload
  FCSChecksum = (FCSChecksum >> 8) ^ fcstab[(FCSChecksum ^ v) & 0xff];
  if (v == 0x7d || v == 0x7e || v == 0x11 || v == 0x12 || v == 0x13) {
    btbuffer[pcktBufPos++] = 0x7d;
    btbuffer[pcktBufPos++] = v ^ 0x20;
  } else {
    btbuffer[pcktBufPos++] = v;
  }
}
// ********************************************************
void write32(uint8_t *btbuffer, uint32_t v) {
  writeByte(btbuffer,(uint8_t)((v >> 0) & 0xFF));
  writeByte(btbuffer,(uint8_t)((v >> 8) & 0xFF));
  writeByte(btbuffer,(uint8_t)((v >> 16) & 0xFF));
  writeByte(btbuffer,(uint8_t)((v >> 24) & 0xFF));
}
// ********************************************************
void write16(uint8_t *btbuffer, uint16_t v) {
  writeByte(btbuffer,(uint8_t)((v >> 0) & 0xFF));
  writeByte(btbuffer,(uint8_t)((v >> 8) & 0xFF));
}
// ********************************************************
void writeArray(uint8_t *btbuffer, const uint8_t bytes[], int loopcount) {
    for (int i = 0; i < loopcount; i++) writeByte(btbuffer, bytes[i]);
}
// ********************************************************
//  writePacket(pcktBuf, 0x0E, 0xA0, 0x0100, 0xFFFF, 0xFFFFFFFF); // anySUSyID, anySerial);
void writePacket(uint8_t *buf, uint8_t longwords, uint8_t ctrl, uint16_t ctrl2, uint16_t dstSUSyID, uint32_t dstSerial) {
  buf[pcktBufPos++] = 0x7E;   //Not included in checksum
  write32(buf, BTH_L2SIGNATURE);
  writeByte(buf, longwords);
  writeByte(buf, ctrl);
  write16(buf, dstSUSyID);
  write32(buf, dstSerial);
  write16(buf, ctrl2);
  write16(buf, AppSUSyID);
  write32(buf, AppSerial);
  write16(buf, ctrl2);
  write16(buf, 0);
  write16(buf, 0);
  write16(buf, pcktID | 0x8000);
}
//-------------------------------------------------------------------------
void writePacketTrailer(uint8_t *btbuffer) {
  FCSChecksum ^= 0xFFFF;
  btbuffer[pcktBufPos++] = FCSChecksum & 0x00FF;
  btbuffer[pcktBufPos++] = (FCSChecksum >> 8) & 0x00FF;
  btbuffer[pcktBufPos++] = 0x7E;  //Trailing byte
}
//-------------------------------------------------------------------------
void writePacketLength(uint8_t *buf) {
  buf[1] = pcktBufPos & 0xFF;         //Lo-Byte
  buf[2] = (pcktBufPos >> 8) & 0xFF;  //Hi-Byte
  buf[3] = buf[0] ^ buf[1] ^ buf[2];      //checksum
}
//-------------------------------------------------------------------------
bool validateChecksum() {
  FCSChecksum = 0xffff;
  //Skip over 0x7e at start and end of packet
  for(int i = 1; i <= pcktBufPos - 4; i++) {
    FCSChecksum = (FCSChecksum >> 8) ^ fcstab[(FCSChecksum ^ pcktBuf[i]) & 0xff];
  }
  FCSChecksum ^= 0xffff;

  if (get_u16(pcktBuf+pcktBufPos-3) == FCSChecksum) {
    return true;
  } else {
    DEBUG1_PRINTF("Invalid validateChecksum 0x%04X not 0x%04X\n", 
    FCSChecksum, get_u16(pcktBuf+pcktBufPos-3));
    return false;
  }
}
