#include "decodeutf8.h"

// source: https://github.com/Bodmer/Adafruit-GFX-Library/blob/master/Adafruit_GFX.cpp
// fork of Adafruit-GFX-Library by Bodmer
// line 1135 of Adafruit_GFX.cpp

/*
https://tools.ietf.org/html/rfc3629
Network Working Group                                         
Request for Comments: 3629  
F. Yergeau
November 2003

UTF-8, a transformation format of ISO 10646


4.  Syntax of UTF-8 Byte Sequences

   For the convenience of implementors using ABNF, a definition of UTF-8
   in ABNF syntax is given here.

   A UTF-8 string is a sequence of octets representing a sequence of UCS
   characters.  An octet sequence is valid UTF-8 only if it matches the
   following syntax, which is derived from the rules for encoding UTF-8
   and is expressed in the ABNF of [RFC2234].

   UTF8-octets = *( UTF8-char )
   UTF8-char   = UTF8-1 / UTF8-2 / UTF8-3 / UTF8-4
   UTF8-1      = %x00-7F
   UTF8-2      = %xC2-DF UTF8-tail
   UTF8-3      = %xE0 %xA0-BF UTF8-tail / %xE1-EC 2( UTF8-tail ) /
                 %xED %x80-9F UTF8-tail / %xEE-EF 2( UTF8-tail )
   UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) / %xF1-F3 3( UTF8-tail ) /
                 %xF4 %x80-8F 2( UTF8-tail )
   UTF8-tail   = %x80-BF

   NOTE -- The authoritative definition of UTF-8 is in [UNICODE].  This
   grammar is believed to describe the same thing Unicode describes, but
   does not claim to be authoritative.  Implementors are urged to rely
   on the authoritative source, rather than on this ABNF.

----------------------------------------------

*/


uint8_t  decoderState = 0;   // UTF-8 decoder state
uint16_t decoderBuffer;      // Unicode code-point buffer


void resetUTF8decoder(void) {
  decoderState = 0;  
}

// Returns Unicode code point in the 0 - 0xFFFE range.  0xFFFF is used to signal 
// that more bytes are needed to complete decoding a multi-byte UTF-8 encoding
//   
// This is just a serial decoder, it does not check to see if the code point is 
// actually assigned to a character in Unicode.
uint16_t decodeUTF8(uint8_t c) {  
 
  if ((c & 0x80) == 0x00) { // 7 bit Unicode Code Point
    decoderState = 0;
    return (uint16_t) c;
  }

  if (decoderState == 0) {

    if ((c & 0xE0) == 0xC0) { // 11 bit Unicode Code Point
        decoderBuffer = ((c & 0x1F)<<6); // Save first 5 bits
        decoderState = 1;
    } else if ((c & 0xF0) == 0xE0) {  // 16 bit Unicode Code Point      {
        decoderBuffer = ((c & 0x0F)<<12);  // Save first 4 bits
        decoderState = 2;
    }    
  
  } else {
      decoderState--;
      if (decoderState == 1) 
        decoderBuffer |= ((c & 0x3F)<<6); // Add next 6 bits of 16 bit code point
      else if (decoderState == 0) {
        decoderBuffer |= (c & 0x3F); // Add last 6 bits of code point (UTF8-tail)
        return decoderBuffer;
      }
  }
  return 0xFFFF; 
}

