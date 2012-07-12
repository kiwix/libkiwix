/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "unaccent.h"

UErrorCode status = U_ZERO_ERROR;
Transliterator *trans = Transliterator::createInstance("Lower; NFD; [:M:] remove; NFC", UTRANS_FORWARD, status);

std::string removeAccents(const std::string &text) {
  ucnv_setDefaultName("UTF-8");
  UnicodeString ustring = UnicodeString(text.c_str());
  trans->transliterate(ustring);
  std::string unaccentedText;
  ustring.toUTF8String(unaccentedText);
  return unaccentedText;
}

void printStringInHexadecimal(UnicodeString s) {
  std::cout << std::showbase << std::hex;
  for (int i=0; i<s.length(); i++) {
    char c = (char)((s.getTerminatedBuffer())[i]);
    if (c & 0x80)
      std::cout << (c & 0xffff) << " ";
    else
      std::cout << c << " ";
  }
  std::cout << std::endl;
}

void printStringInHexadecimal(const char *s) {
  std::cout << std::showbase << std::hex;
  for (char const* pc = s; *pc; ++pc) {
    if (*pc & 0x80)
      std::cout << (*pc & 0xffff);
    else
      std::cout << *pc;
    std::cout << ' ';
  }
  std::cout << std::endl;
}
