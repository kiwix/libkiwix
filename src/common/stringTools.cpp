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

std::vector<std::string> split(const std::string & str,
                                      const std::string & delims=" *-")
{
  string::size_type lastPos = str.find_first_not_of(delims, 0);
  string::size_type pos = str.find_first_of(delims, lastPos);
  vector<string> tokens;
 
  while (string::npos != pos || string::npos != lastPos)
    {
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      lastPos = str.find_first_not_of(delims, pos);
      pos     = str.find_first_of(delims, lastPos);
    }
 
  return tokens;
}

std::vector<std::string> split(const char* lhs, const char* rhs){
  const std::string m1 (lhs), m2 (rhs);
  return split(m1, m2);
}

std::vector<std::string> split(const char* lhs, const std::string& rhs){
  return split(lhs, rhs.c_str());
}

std::vector<std::string> split(const std::string& lhs, const char* rhs){
  return split(lhs.c_str(), rhs);
}
