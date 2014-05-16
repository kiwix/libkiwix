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

#include "componentTools.h"

const char *nsStringToCString(const nsAString &str) {
  const char *cStr;
  nsCString tmpStr;

#ifdef _WIN32
  LossyCopyUTF16toASCII(str, tmpStr);
#else
  CopyUTF16toUTF8(str, tmpStr);
#endif

  NS_CStringGetData(tmpStr, &cStr);

#ifdef _WIN32
  return _strdup(cStr);
#else
  return strdup(cStr);
#endif
}

std::string nsStringToString(const nsEmbedString &str) {
#ifdef _WIN32
  PRUnichar *start = (PRUnichar *)str.get();
  PRUnichar *end = start +  str.Length();
  wchar_t wca[4096];
  wchar_t *wstart = wca;
  wchar_t *wpr = wstart;
        
  for(; start < end; ++start)
    {
      *wstart = (wchar_t) *start;
      ++wstart;
    }
  *wstart = 0;

  std::string ptr;
  ptr.resize(4096);
  size_t size = wcstombs((char*)ptr.data(), wpr, 4096);
  ptr.resize(size);

  return ptr;
#else
  const char *cStr;
  nsCString tmpStr;

  CopyUTF16toUTF8(str, tmpStr);
  NS_CStringGetData(tmpStr, &cStr);
  return std::string(cStr);
#endif

}

const char *nsStringToUTF8(const nsAString &str) {
  const char *cStr;
  nsCString tmpStr;
  CopyUTF16toUTF8(str, tmpStr);
  NS_CStringGetData(tmpStr, &cStr);

#ifdef _WIN32
  return _strdup(cStr);
#else
  return strdup(cStr);
#endif
}
