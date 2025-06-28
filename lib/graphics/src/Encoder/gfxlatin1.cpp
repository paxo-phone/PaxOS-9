#include "gfxlatin1.h"

#include "decodeutf8.h"

#include <cstring>

// In place conversion of a UTF8 string to extended ASCII string (ASCII is
// shorter!)
void latin1tocp(char* s)
{
    int k = 0;
    uint16_t ucs2;
    resetUTF8decoder();
    for (int i = 0; i < strlen(s); i++)
    {
        ucs2 = decodeUTF8(s[i]);
        if (ucs2 <= 0x7F)
        {
            s[k++] = (char) ucs2;
        }
        else if (0xA0 <= ucs2 && ucs2 <= 0xFF)
        {
            s[k++] = (char) (ucs2 - 32);
        }
#ifdef SHOW_UNMAPPED_CODES
        else if (ucs2 < 0xFFFF)
        { //(0xFF < ucs2 && ucs2 < 0xFFFF)
            s[k++] = (char) 0x7F;
        }
#endif
    }
    s[k] = 0;
}
