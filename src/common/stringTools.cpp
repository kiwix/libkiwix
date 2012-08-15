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

#include "stringTools.h"

/* Prepare integer for display */
std::string kiwix::beautifyInteger(const unsigned int number) {
  std::stringstream numberStream;
  numberStream << number;
  std::string numberString = numberStream.str();
  
  signed int offset = numberString.size() - 3;
  while (offset > 0) {
    numberString.insert(offset, ",");
    offset -= 3;
  }
  
  return numberString;
}

/* Split string in a token array */
std::vector<std::string> kiwix::split(const std::string & str,
                                      const std::string & delims=" *-")
{
  std::string::size_type lastPos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, lastPos);
  std::vector<std::string> tokens;
 
  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      lastPos = str.find_first_not_of(delims, pos);
      pos     = str.find_first_of(delims, lastPos);
    }
 
  return tokens;
}

std::vector<std::string> kiwix::split(const char* lhs, const char* rhs){
  const std::string m1 (lhs), m2 (rhs);
  return split(m1, m2);
}

std::vector<std::string> kiwix::split(const char* lhs, const std::string& rhs){
  return split(lhs, rhs.c_str());
}

std::vector<std::string> kiwix::split(const std::string& lhs, const char* rhs){
  return split(lhs.c_str(), rhs);
}

std::string kiwix::removeAccents(const std::string &text) {
  ucnv_setDefaultName("UTF-8");
  UErrorCode status = U_ZERO_ERROR;
  Transliterator *removeAccentsTrans = Transliterator::createInstance("Lower; NFD; [:M:] remove; NFC", UTRANS_FORWARD, status);
  UnicodeString ustring = UnicodeString(text.c_str());
  removeAccentsTrans->transliterate(ustring);
  std::string unaccentedText;
  ustring.toUTF8String(unaccentedText);
  return unaccentedText;
}

std::string kiwix::ucFirst (const std::string &word) {
  if (word.empty())
    return "";

  std::string ucFirstWord;
  UnicodeString firstLetter = UnicodeString(word.substr(0, 1).c_str());
  UnicodeString ucFirstLetter = firstLetter.toUpper();
  ucFirstLetter.toUTF8String(ucFirstWord);
  ucFirstWord += word.substr(1);
  return ucFirstWord;
}

std::string kiwix::lcFirst (const std::string &word) {
  if (word.empty())
    return "";

  std::string ucFirstWord;
  UnicodeString firstLetter = UnicodeString(word.substr(0, 1).c_str());
  UnicodeString ucFirstLetter = firstLetter.toLower();
  ucFirstLetter.toUTF8String(ucFirstWord);
  ucFirstWord += word.substr(1);
  return ucFirstWord;
}

void kiwix::printStringInHexadecimal(UnicodeString s) {
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

void kiwix::printStringInHexadecimal(const char *s) {
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


