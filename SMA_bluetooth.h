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

#ifndef SMA_BLUETOOTH_SEEN
#define SMA_BLUETOOTH_SEEN

uint8_t sixzeros[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t sixff[6]   = {0xff,0xff,0xff,0xff,0xff,0xff};

//unsigned char EspBTAddress[6] = {0xE6,0x72,0xCC,0xD1,0x08,0xF0}; // BT address ESP32 F0:08:D1:CC:72:E6
//                        \|E6\|72\|CC\|D1\|08\|F0 };  // BT address  ESP32 F0:08:D1:CC:72:E6
//                        \|d3\|eb\|29\|25\|80\|00 };  // //SMC 6000: 00:80:25:29:eb:d3

#define BTH_L2SIGNATURE 0x656003FF

#endif //SMA_BLUETOOTH_SEEN
