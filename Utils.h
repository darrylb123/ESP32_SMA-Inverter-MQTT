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

#ifndef SMA_UTILS_SEEN
#define SMA_UTILS_SEEN


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

#endif //SMA_UTILS_SEEN
