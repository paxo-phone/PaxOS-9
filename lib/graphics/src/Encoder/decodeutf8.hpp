#ifndef DECODEUTF8_H
#define DECODEUTF8_H

#include "stdint.h"

#include <string>

void resetUTF8decoder(void);
uint16_t decodeUTF8(uint8_t c);
std::string decodeString(std::string& code);

#endif
