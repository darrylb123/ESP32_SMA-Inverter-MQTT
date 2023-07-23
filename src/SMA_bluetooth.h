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



#ifndef SMA_BLUETOOTH.H
#define SMA_BLUETOOTH.H


#include <Arduino.h>

//unsigned char EspBTAddress[6] = {0xE6,0x72,0xCC,0xD1,0x08,0xF0}; // BT address ESP32 F0:08:D1:CC:72:E6
//                        \|E6\|72\|CC\|D1\|08\|F0 };  // BT address  ESP32 F0:08:D1:CC:72:E6
//                        \|d3\|eb\|29\|25\|80\|00 };  // //SMC 6000: 00:80:25:29:eb:d3

#define BTH_L2SIGNATURE 0x656003FF


//Prototypes
uint8_t BTgetByte();
void BTsendPacket( uint8_t *btbuffer );
void writeByte(uint8_t *btbuffer, uint8_t v);
void write32(uint8_t *btbuffer, uint32_t v);
void write16(uint8_t *btbuffer, uint16_t v);
void writeArray(uint8_t *btbuffer, const uint8_t bytes[], int loopcount);
void writePacket(uint8_t *buf, uint8_t longwords, uint8_t ctrl, uint16_t ctrl2, uint16_t dstSUSyID, uint32_t dstSerial);
void writePacketTrailer(uint8_t *btbuffer);
void writePacketLength(uint8_t *buf);
bool validateChecksum();

#endif
