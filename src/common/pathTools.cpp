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

#include "pathTools.h"

const char *nsStringToCString(const nsAString &str) {
  const char *cStr;
  nsCString tmpStr;

#ifdef _WIN32
  LossyCopyUTF16toASCII(str, tmpStr):
#else
  CopyUTF16toUTF8(str, tmpStr);
#endif

  NS_CStringGetData(tmpStr, &cStr);
  return cStr;
}
