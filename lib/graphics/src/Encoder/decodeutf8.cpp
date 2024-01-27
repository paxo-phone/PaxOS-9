#include "decodeutf8.hpp"

#include <stdint.h>
#include <vector>

#include "../fonts/Arial-charset.h"

uint8_t  decoderState = 0;   // UTF-8 decoder state
uint16_t decoderBuffer;      // Unicode code-point buffer

void resetUTF8decoder(void) {
  decoderState = 0;  
}

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

std::string decodeString(std::string &code)
{
  resetUTF8decoder();

  std::string code_8;
  std::vector<uint16_t> code_16;

  for (int i = 0; i < code.size(); i++)
    code_16.push_back(decodeUTF8(code[i]));

  for (int i = 0; i < code.size(); i++)
  {
    bool result = false;
    for (int j = 0; j < FRCharcount; j++)
    {
      if(code_16[i] == FRCharset[j].UTF)
      {
          result = true;
          code_8.push_back(FRCharset[j].latin);
          break;
      }
    }

    if(!result && char(code_16[i]) != -1)
    {
        code_8.push_back(code_16[i]);
    }
  }

  return code_8;
}