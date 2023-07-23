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

#ifndef SMA_UTILS.H
#define SMA_UTILS.H
// Debug ------------------------------------------------
#if (DEBUG_SMA > 0)
  #define  DEBUG1_PRINTF   Serial.printf
  #define  DEBUG1_PRINT    Serial.print
  #define  DEBUG1_PRINTLN  Serial.println
#else
  #define  DEBUG1_PRINTF(...)  /**/ 
  #define  DEBUG1_PRINT(...)   /**/  
  #define  DEBUG1_PRINTLN(...) /**/
#endif

#if (DEBUG_SMA > 1)
  #define  DEBUG2_PRINTF   Serial.printf
  #define  DEBUG2_PRINT    Serial.print
  #define  DEBUG2_PRINTLN  Serial.println
#else
  #define  DEBUG2_PRINTF(...)  /**/ 
  #define  DEBUG2_PRINT(...)   /**/  
  #define  DEBUG2_PRINTLN(...) /**/
#endif

#if (DEBUG_SMA > 2)
  #define  DEBUG3_PRINTF   Serial.printf
  #define  DEBUG3_PRINT    Serial.print
  #define  DEBUG3_PRINTLN  Serial.println
#else
  #define  DEBUG3_PRINTF(...)  /**/ 
  #define  DEBUG3_PRINT(...)   /**/  
  #define  DEBUG3_PRINTLN(...) /**/
#endif

// Prototypes
void HexDump(uint8_t *buf, int count, int radix, uint8_t c);
uint8_t printUnixTime(char *buf, time_t t);
uint16_t get_u16(uint8_t *buf);
uint32_t get_u32(uint8_t *buf);
uint64_t get_u64(uint8_t *buf);

#endif