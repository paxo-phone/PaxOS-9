/*
 * Converting UTF-8 encoded strings to GFX Latin 1 encoded strings
 *
 * The GFX Latin 1 encoding is described in
 *   https://sigmdel.ca/michel/program/misc/gfxfont_8bit_02_en.html
 *
 *
 * To display the converted string, an Adafruit GFXfont containing the 192
 * printable ISO 8859-1 characters corresponding to the GFX Latin 1 character
 * set must be used. See fontconvert8 to obtain such a font from TTF fonts
 *   https://sigmdel.ca/michel/program/misc/gfxfont_8bit_en.html#fontconvert8
 *
 */

#ifndef GFXLATIN1_H
#define GFXLATIN1_H

// Convert a UTF-8 encoded string to a GFX Latin 1 encoded string
// Be careful, the in-situ conversion will "destroy" the UTF-8 string s.
void latin1tocp(char* s);

#endif
