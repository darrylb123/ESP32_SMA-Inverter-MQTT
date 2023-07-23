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

#include "Utils.h"

void HexDump(uint8_t *buf, int count, int radix, uint8_t c) {
  int i, j;
  DEBUG2_PRINTF("\n---%c----:", c);
  for (i=0; i < radix; i++) DEBUG2_PRINTF(" %02X", i);
  for (i = 0, j = 0; i < count; i++) {
    if (j % radix == 0) {
      DEBUG2_PRINTF("\n%c-%06d: ", c, j);
    }
    DEBUG2_PRINTF("%02X ", buf[i]);
    j++;
  }
  DEBUG2_PRINT("\n");
}
//-----------------------------------------------------
 uint8_t printUnixTime(char *buf, time_t t) {
    uint32_t a, b, c, d, e, f;
    //Negative Unix time values are not supported
    if(t < 1) t = 0;
  
    //Retrieve hours, minutes and seconds
    uint8_t seconds = t % 60;
    t /= 60;
    uint8_t minutes = t % 60;
    t /= 60;
    uint8_t hours = t % 24;
    t /= 24;
  
    //Convert Unix time to date
    a = (uint32_t) ((4 * t + 102032) / 146097 + 15);
    b = (uint32_t) (t + 2442113 + a - (a / 4));
    c = (20 * b - 2442) / 7305;
    d = b - 365 * c - (c / 4);
    e = d * 1000 / 30601;
    f = d - e * 30 - e * 601 / 1000;
  
    //January and February are counted as months 13 and 14 of the previous year
    if(e <= 13) {
       c -= 4716;
       e -= 1;
    } else {
       c -= 4715;
       e -= 13;
    }
    //Retrieve year, month and day
    uint16_t year = c;
    uint8_t  month = e;
    uint8_t  day = f;
    return snprintf(buf, 31,"%02d.%02d.%d %02d:%02d:%02d",day, month, year, hours, minutes, seconds);
    //DEBUG1_PRINTF(" GMT: %02d.%02d.%d %02d:%02d:%02d",day, month, year, hours, minutes, seconds);
 }

//-----------------------------------------------------
uint16_t get_u16(uint8_t *buf) {
    register uint16_t shrt = 0;
    shrt += *(buf+1);
    shrt <<= 8;
    shrt += *(buf);
    return shrt;
}
uint32_t get_u32(uint8_t *buf) {
    register uint32_t lng = 0;
    lng += *(buf+3);
    lng <<= 8;
    lng += *(buf+2);
    lng <<= 8;
    lng += *(buf+1);
    lng <<= 8;
    lng += *(buf);
    return lng;
}
uint64_t get_u64(uint8_t *buf) {
    register uint64_t lnglng = 0;
    lnglng += *(buf+7);
    lnglng <<= 8;
    lnglng += *(buf+6);
    lnglng <<= 8;
    lnglng += *(buf+5);
    lnglng <<= 8;
    lnglng += *(buf+4);
    lnglng <<= 8;
    lnglng += *(buf+3);
    lnglng <<= 8;
    lnglng += *(buf+2);
    lnglng <<= 8;
    lnglng += *(buf+1);
    lnglng <<= 8;
    lnglng += *(buf);
    return lnglng;
}
